#include <Arduino.h>
#include <simplifiedEncoder.h>
#include <stateMachine.h>
#include <rtcManager.h>
#include <Button.h>
#include <limitTemperature.h>
#include <TM1637Display.h>
#include <LEDcontroller.h>
#include <main.h>


#define TEMP_CLK 16
#define TEMP_DIO 17
#define CLOCK_CLK 18
#define CLOCK_DIO 19
TM1637Display temperatureDisplay(TEMP_CLK, TEMP_DIO);
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
uint16_t displayedTime = 0;

uint8_t displayBrightness = 7;

void setup() {
    Serial.begin(115200);
    buttonEncoder.begin();
    buttonSetTime.begin();
    buttonSetLowTemp.begin();
    buttonSetProofingTemperature.begin();

    pinMode(LED_BUILTIN, OUTPUT);
    ledCold.init();
    ledHot.init();
    ledProofing.init();

    limitTemperature.init();

    if (rtcManager.init()) {
        changeState(STATE_WAITING);
    }
    else {
        Serial.println("RTC lost power, need to set the time!");
        changeState(STATE_TIME_UNSET);
    }
}

bool handleButtonSetLowTemp(const int8_t encoderMovement) {
    bool hasActed = false;
    if(buttonSetLowTemp.read() == Button::PRESSED && encoderMovement) {
        limitTemperature.setLowTemp(encoderMovement);
        hasActed = true;
    }
    else if (buttonSetLowTemp.released()) {
        limitTemperature.storeLowTemp();
        hasActed = true;
    }
    return hasActed;
}

bool handleButtonSetProofingTemperature(const int8_t encoderMovement) {
    bool hasActed = false;
    if(buttonSetProofingTemperature.read() == Button::PRESSED && encoderMovement) {
        limitTemperature.setProofingTemperature(encoderMovement);
        const int8_t temperature = limitTemperature.getProofingTemperature();
        temperatureDisplay.showNumberDec(temperature);
        Serial.print("Proofing temperature: ");
        Serial.println(temperature);
        hasActed = true;
    }
    else if (buttonSetProofingTemperature.released()) {
        limitTemperature.storeProofingTemperature();
        hasActed = true;
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
        Serial.print("New time: ");
        Serial.println(tempTime);
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
        Serial.println(displayBrightness);
        clockDisplay.setBrightness(displayBrightness);
        temperatureDisplay.setBrightness(displayBrightness);

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

void displayTime(const uint16_t time) {
    const uint8_t dots = 0b01000000;
    clockDisplay.showNumberDecEx(time, dots);
}

void loop() {
    const int8_t encoderMovement = encoder.getRelativeMovement();
    bool encoderValueUsed = handleSetBrightness(encoderMovement);
    if (!encoderValueUsed) {
        handleButtonSetLowTemp(encoderMovement);
    }
    if (!encoderValueUsed) {
        encoderValueUsed = handleButtonSetProofingTemperature(encoderMovement);
    }
    if (!encoderValueUsed) {
        encoderValueUsed = handleButtonSetTime(encoderMovement);
    }
    if (!encoderValueUsed) {
        stateMachineReact(encoderMovement);
    }
}

float getTemperature() {
    return 12.35;
}

void blinkCountdownLED() {
    const uint32_t currentMillis = millis();
 
    if(currentMillis - lastBlinkingTime > 250) {
        lastBlinkingTime = currentMillis;   

        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
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
    Serial.println("Setting the start time");
}

void stateWaitingAct(const int8_t encoderMovement) {
    if(encoderMovement) {
        runIfNewState(stateWaitingInit);
        rtcManager.setStartTime(encoder.getAcceleratedRelativeMovement(encoderMovement));
        const uint16_t newStartTime = rtcManager.getStartTime();
        displayTime(newStartTime);
        Serial.print("New start time ");
        Serial.println(newStartTime);
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
            Serial.print(rtcTime);
            Serial.print(": ");
            printStateToSerial();
            Serial.println("");
            displayedTime = rtcTime;
        }
        previousTickTime = currentMillis;
    }
}

void stateTimeUnsetInit() {
    rtcManager.initSetTime();
}
void stateTimeUnsetAct(const int8_t encoderMovement){
    if(encoderMovement) {
        runIfNewState(stateTimeUnsetInit);
        rtcManager.setTime(encoder.getAcceleratedRelativeMovement(encoderMovement));
        const uint16_t tempTime = rtcManager.getTempTime();
        displayTime(tempTime);
        Serial.println(tempTime);
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
        Serial.println(startTime);
    }
    const uint32_t currentMillis = millis();
    if(currentMillis - previousTickTime > 1000) {
        const float currentTemperature = getTemperature();
        if(limitTemperature.lowTemperatureTooLow(currentTemperature)) {
            switchColdOff();
        }
        else if(limitTemperature.lowTemperatureTooHigh(currentTemperature)) {
            switchColdOn();
        }
        const uint16_t timeLeft = rtcManager.getTimeLeftInCountdown();
        if (timeLeft != displayedTime) {
            displayTime(timeLeft);
            Serial.print("Time left: ");
            Serial.println(timeLeft);
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
            Serial.print("Time proofing: ");
            Serial.println(timeProofing);
            displayedTime = timeProofing;
        }
        const float currentTemperature = getTemperature();
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
    Serial.println("Start heating!!!");
    digitalWrite(LED_BUILTIN, LOW);
    ledCold.off();
    ledProofing.on();
}
