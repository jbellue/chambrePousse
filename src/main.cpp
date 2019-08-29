#include <debug_macros.h>
#include <Arduino.h>
#include <simplifiedEncoder.h>
#include <stateMachine.h>
#include <rtcManager.h>
#include <Button.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <limitTemperature.h>
#include <TM1637Display.h>
#include <LEDcontroller.h>
#include <main.h>


#define ONE_WIRE_BUS 12

// Resolution goes fom 9 to 12, with these values:
//          Accuracy (°C)   Time to get a temperature (ms)
//  9 bit   0.5             93.75
// 10 bit   0.25            187.5
// 11 bit   0.125           375
// 12 bit   0.0625          750
// So it's a trade-off... 
#define THERMOMETER_RESOLUTION 11
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature thermometer(&oneWire);

#define TEMP_CLK 14
#define TEMP_DIO 15
#define CLOCK_CLK 16
#define CLOCK_DIO 17
TM1637Display temperatureDisplay(TEMP_CLK, TEMP_DIO);
const uint8_t temperatureSymbol = SEG_A | SEG_B | SEG_F | SEG_G;
TM1637Display clockDisplay(CLOCK_CLK, CLOCK_DIO);

// Use pins 2 and 3 because they're the only two with
// interrupt on the nano
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
SimplifiedEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

#define PIN_BTN_ENCODER 4
#define PIN_BTN_SET_TIME 5
#define PIN_BTN_SET_LOW_TEMP 6
#define PIN_BTN_SET_HIGH_TEMP 7

// Need to tweak the resistor values for the LEDs when I
// actually get them to get consistent brightness
#define PIN_LED_PROOFING 9
LED ledProofing(PIN_LED_PROOFING);
#define PIN_LED_COLD 10
LED ledCold(PIN_LED_COLD);
#define PIN_LED_HOT 11
LED ledHot(PIN_LED_HOT);

Button buttonEncoder(PIN_BTN_ENCODER);
Button buttonSetTime(PIN_BTN_SET_TIME);
Button buttonSetLowTemp(PIN_BTN_SET_LOW_TEMP);
Button buttonSetProofingTemperature(PIN_BTN_SET_HIGH_TEMP);

LimitTemperature limitTemperature;
RTCManager rtcManager;

uint32_t previousTickTime = 0;
uint32_t lastBlinkingTime = 0;
uint32_t lastTemperatureUpdate = 0;
uint8_t countdownPattern = 0b00001000;
uint16_t displayedTime = 0;
int16_t displayedTemperature = 0;
float currentTemperature = 0;
bool displayIsShowingDecimals = true;

uint8_t displayBrightness = 7;

void setup() {
    DebugPrintBegin(115200);
    buttonEncoder.begin();
    buttonSetTime.begin();
    buttonSetLowTemp.begin();
    buttonSetProofingTemperature.begin();

    clockDisplay.setBrightness(displayBrightness);
    temperatureDisplay.setBrightness(displayBrightness);

    pinMode(LED_BUILTIN, OUTPUT);
    ledCold.init();
    ledHot.init();
    ledProofing.init();

    limitTemperature.init();

    thermometer.begin();
    thermometer.setResolution(THERMOMETER_RESOLUTION);

    switch(rtcManager.init()) {
        case RTCManager::initReturn_t::success:
            changeState(STATE_WAITING);
            break;
        case RTCManager::initReturn_t::lostPower:
            changeState(STATE_TIME_UNSET);
            break;
        case RTCManager::initReturn_t::notFound:
            DebugPrintlnFull("Couldn't find RTC module.");
            // intentional fallthrough
        default:
            DebugPrintlnFull("Error while initialising RTC module. Aborting start.");
            while(1);
    }
}

bool handleButtonSetLowTemp(const int8_t encoderMovement) {
    bool hasActed = false;
    if(buttonSetLowTemp.read() == Button::PRESSED) {
        if (encoderMovement) {
            limitTemperature.setLowTemp(encoderMovement);
        }
        hasActed = true;
        displayTemperature(limitTemperature.getLowTemp(), false, false);
    }
    else if (buttonSetLowTemp.released()) {
        limitTemperature.storeLowTemp();
    }
    return hasActed;
}

bool handleButtonSetProofingTemperature(const int8_t encoderMovement) {
    bool hasActed = false;
    if(buttonSetProofingTemperature.read() == Button::PRESSED) {
        if (encoderMovement) {
            limitTemperature.setProofingTemperature(encoderMovement);
        }
        hasActed = true;
        displayTemperature(limitTemperature.getProofingTemperature(), false, false);
    }
    else if (buttonSetProofingTemperature.released()) {
        limitTemperature.storeProofingTemperature();
    }
    return hasActed;
}

bool handleButtonSetTime(const int8_t encoderMovement) {
    bool hasActed = false;
    if(buttonSetTime.pressed()) {
        rtcManager.initSetTime();
        hasActed = true;
    }
    else if(buttonSetTime.read() == Button::PRESSED && encoderMovement) {
        rtcManager.setTime(encoder.getAcceleratedRelativeMovement(encoderMovement));
        const uint16_t tempTime = rtcManager.getTempTime();
        displayTime(tempTime);
        DebugPrintFull("New time: ");
        DebugPrintln(tempTime);
        hasActed = true;
    }
    else if (buttonSetTime.released()) {
        rtcManager.finishTimeSet();
        hasActed = true;
    }
    return hasActed;
}

bool handleSetBrightness(const int8_t encoderMovement) {
    bool hasActed = false;
    if(buttonSetProofingTemperature.read() == Button::PRESSED
        && buttonSetLowTemp.read() == Button::PRESSED
        && encoderMovement) {
        // the displays expect values 0-7
        displayBrightness = constrain(displayBrightness + encoderMovement, 0, 7);
        DebugPrintlnFull(displayBrightness);

        clockDisplay.setBrightness(displayBrightness);
        displayTime(displayedTime);
        temperatureDisplay.setBrightness(displayBrightness);
        displayTemperature(displayedTemperature, true);

        ledCold.setBrightness(displayBrightness);
        ledHot.setBrightness(displayBrightness);
        ledProofing.setBrightness(displayBrightness);
        hasActed = true;
    }
    else if (buttonSetProofingTemperature.released() && buttonSetLowTemp.released()) {
        hasActed = true;
    }
    return hasActed;
}

void displayTime(const uint16_t time, const uint8_t dots) {
    clockDisplay.showNumberDecEx(time, dots);
}

void displayTemperature(const float temp, bool forceRefresh, bool showDecimal) {
    if (forceRefresh || (int)((temp * 10) + 0.5) != displayedTemperature * 10 || displayIsShowingDecimals != showDecimal) {
        displayIsShowingDecimals = showDecimal;
        int16_t tempToDisplay;
        if (showDecimal) {
            tempToDisplay = temp * 10 + 0.5; //poor man's round
        }
        else {
            tempToDisplay = temp;
        }
        temperatureDisplay.showNumberDecEx(tempToDisplay, showDecimal ? 0b01000000:0, false, 3, 0);
        temperatureDisplay.setSegments(&temperatureSymbol, 1, 3);

        displayedTemperature = tempToDisplay;
    }
}

void loop() {
    const int8_t encoderMovement = encoder.getRelativeMovement();
    bool encoderValueUsed = handleSetBrightness(encoderMovement);
    bool temperatureIsBeingSet = false;
    if (!encoderValueUsed) {
        encoderValueUsed = handleButtonSetProofingTemperature(encoderMovement);
        temperatureIsBeingSet |= encoderValueUsed;
    }
    if (!encoderValueUsed) {
        encoderValueUsed = handleButtonSetLowTemp(encoderMovement);
        temperatureIsBeingSet |= encoderValueUsed;
    }
    if (!encoderValueUsed) {
        encoderValueUsed = handleButtonSetTime(encoderMovement);
    }
    if (!encoderValueUsed) {
        stateMachineReact(encoderMovement);
    }
    if (!temperatureIsBeingSet) {
        // display the temperature every second
        const uint32_t currentMillis = millis();
        if(currentMillis - lastTemperatureUpdate > 1000) {
            currentTemperature = getTemperature();
            displayTemperature(currentTemperature);
            lastTemperatureUpdate = currentMillis;
        }
    }
}

float getTemperature() {
    thermometer.requestTemperatures();
    const float temp = thermometer.getTempCByIndex(0);
    DebugPrintFull("temperature: ");
    DebugPrintln(temp);
    return temp;
}

uint8_t rotateBits(const uint8_t n) {
  return ((n & 1) << 3) | (n >> 1);
}

void blinkCountdownLED() {
    const uint32_t currentMillis = millis();
 
    if(currentMillis - lastBlinkingTime > 250) {
        countdownPattern = rotateBits(countdownPattern);
        displayTime(displayedTime, countdownPattern);
        lastBlinkingTime = currentMillis;
    }
}

void switchColdOn() {
    ledCold.on();
}
void switchColdOff() {
    ledCold.off();
}
void switchHotOn() {
    ledHot.on();
}
void switchHotOff() {
    ledHot.off();
}

void stateWaitingInit() {
    rtcManager.initSetStartTime();
    DebugPrintlnFull("Setting the start time");
}

void stateWaitingAct(const int8_t encoderMovement) {
    if(encoderMovement) {
        runIfNewState(stateWaitingInit);
        rtcManager.setStartTime(encoder.getAcceleratedRelativeMovement(encoderMovement));
        const uint16_t newStartTime = rtcManager.getStartTime();
        displayTime(newStartTime);
        DebugPrintFull("New start time ");
        DebugPrintln(newStartTime);
    }
    else if (buttonEncoder.pressed()) {
        runIfNewState(stateWaitingInit);
        rtcManager.finishStartTimeSet();
        changeState(STATE_COUNTDOWN);
    }
    const uint32_t currentMillis = millis();
    if(currentMillis - previousTickTime > 2000) {
        const uint16_t rtcTime = rtcManager.getRTCTime();
        if (rtcTime != displayedTime) {
            displayTime(rtcTime);
            DebugPrintFull(rtcTime);
            DebugPrint(": ");
            printStateToSerial();
            DebugPrintln("");
            displayedTime = rtcTime;
        }
        previousTickTime = currentMillis;
    }
}

void stateTimeUnsetInit() {
    rtcManager.initSetTime();
}
void stateTimeUnsetAct(const int8_t encoderMovement){
    // TODO blink the display?
    // Just blink all the dots ?
    // Decide.
    if(encoderMovement) {
        runIfNewState(stateTimeUnsetInit);
        rtcManager.setTime(encoder.getAcceleratedRelativeMovement(encoderMovement));
        const uint16_t tempTime = rtcManager.getTempTime();
        displayTime(tempTime);
        DebugPrintlnFull(tempTime);
    }
    else if (buttonEncoder.pressed()) {
        runIfNewState(stateTimeUnsetInit);
        rtcManager.finishTimeSet();
        changeState(STATE_WAITING);
    }
}

void stateCountdownAct(const int8_t encoderMovement) {
    blinkCountdownLED();
    if(encoderMovement) {
        rtcManager.setStartTime(encoder.getAcceleratedRelativeMovement(encoderMovement));
        const uint16_t startTime = rtcManager.getStartTime();
        displayTime(startTime);
        displayedTime = startTime;
        DebugPrintlnFull(startTime);
    }
    const uint32_t currentMillis = millis();
    if(currentMillis - previousTickTime > 1000) {
        if(limitTemperature.lowTemperatureTooLow(currentTemperature)) {
            switchColdOff();
        }
        else if(limitTemperature.lowTemperatureTooHigh(currentTemperature)) {
            switchColdOn();
        }
        const uint16_t timeLeft = rtcManager.getTimeLeftInCountdown();
        if (timeLeft != displayedTime) {
            displayTime(timeLeft);
            DebugPrintFull("Time left: ");
            DebugPrintln(timeLeft);
            displayedTime = timeLeft;
        }
        previousTickTime = currentMillis;
    }
    if (rtcManager.countdownElapsed()) {
        changeState(STATE_PROOFING);
    }
}

void stateProofingAct(int8_t encoderMovement) {
    (void) encoderMovement; // unused parameter...
    runIfNewState(stateProofingInit);

    const uint32_t currentMillis = millis();
    if(currentMillis - previousTickTime > 1000) {
        const uint16_t timeProofing = rtcManager.getTimeProofing();
        if (timeProofing != displayedTime) {
            displayTime(timeProofing);
            DebugPrintFull("Time proofing: ");
            DebugPrintln(timeProofing);
            displayedTime = timeProofing;
        }
        if(limitTemperature.proofingTemperatureTooLow(currentTemperature)) {
            switchHotOn();
        }
        else if(limitTemperature.proofingTemperatureTooHigh(currentTemperature)) {
            switchHotOff();
        }
        previousTickTime = currentMillis;
    }
}

void stateProofingInit() {
    digitalWrite(LED_BUILTIN, LOW);
    ledCold.off();
    ledProofing.on();
}
