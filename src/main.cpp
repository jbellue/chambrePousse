#include <Arduino.h>
#include <Encoder.h>
#include <simplifiedEncoder.h>
#include <stateMachine.h>
#include <rtcManager.h>
#include <Button.h>
#include <limitTemperature.h>
#include <main.h>

// Use pins 2 and 3 because they're the only two with
// interrupt on the nano
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
SimplifiedEncoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

#define PIN_BTN_ENCODER 4
#define PIN_BTN_SET_TIME 5
#define PIN_BTN_SET_LOW_TEMP 6
#define PIN_BTN_SET_HIGH_TEMP 7

#define PIN_LED_PROOFING 8
#define PIN_LED_COLD 10
#define PIN_LED_HOT 11

Button buttonEncoder(PIN_BTN_ENCODER);
Button buttonSetTime(PIN_BTN_SET_TIME);
Button buttonSetLowTemp(PIN_BTN_SET_LOW_TEMP);
Button buttonSetProofingTemperature(PIN_BTN_SET_HIGH_TEMP);

LimitTemperature limitTemperature;
RTCManager rtcManager;

uint32_t previousMillis = 0;
uint32_t previousTickTime = 0;
uint32_t lastBlinkingTime = 0;

void setup() {
    Serial.begin(115200);
    buttonEncoder.begin();
    buttonSetTime.begin();
    buttonSetLowTemp.begin();
    buttonSetProofingTemperature.begin();


    pinMode(PIN_LED_PROOFING, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_LED_COLD, OUTPUT);
    pinMode(PIN_LED_HOT, OUTPUT);

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
        rtcManager.printTempTime();
        hasActed = true;
    }
    else if (buttonSetTime.released()) {
        rtcManager.finishTimeSet();
        hasActed = true;
    }
    return hasActed;
}

void loop() {
    const int8_t encoderMovement = encoder.getRelativeMovement();
    bool encoderValueUsed = handleButtonSetLowTemp(encoderMovement);
    if (!encoderValueUsed) {
        encoderValueUsed = handleButtonSetProofingTemperature(encoderMovement);
    }
    if (!encoderValueUsed) {
        encoderValueUsed = handleButtonSetTime(encoderMovement);
    }
    if (!encoderValueUsed) {
        stateMachineReact(encoderMovement);
    }

    const uint32_t currentMillis = millis();
    if(currentMillis - previousMillis > 2000) {
        rtcManager.printRTCTime();
        Serial.print(": ");
        printStateToSerial();
        Serial.println("");
        previousMillis = currentMillis;
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

void stateWaitingInit() {
    rtcManager.initSetStartTime();
    Serial.println("Setting the start time");
}

void stateWaitingAct(const int8_t encoderMovement) {
    if(encoderMovement) {
        runIfNewState(stateWaitingInit);
        rtcManager.setStartTime(encoder.getAcceleratedRelativeMovement(encoderMovement));
        rtcManager.printStartTime();
    }
    else if (buttonEncoder.pressed()) {
        runIfNewState(stateWaitingInit);
        rtcManager.finishStartTimeSet();
        changeState(STATE_COUNTDOWN);
    }
}

void stateTimeUnsetInit() {
    rtcManager.initSetTime();
}
void stateTimeUnsetAct(const int8_t encoderMovement){
    if(encoderMovement) {
        runIfNewState(stateTimeUnsetInit);
        rtcManager.setTime(encoder.getAcceleratedRelativeMovement(encoderMovement));
        rtcManager.printTempTime();
    }
    else if (buttonEncoder.pressed()) {
        runIfNewState(stateTimeUnsetInit);
        rtcManager.finishTimeSet();
        changeState(STATE_WAITING);
    }
}


void stateCountdownInit() {
    digitalWrite(PIN_LED_COLD, HIGH);
}
void stateCountdownAct(const int8_t encoderMovement) {
    blinkCountdownLED();
    if(encoderMovement) {
        rtcManager.setStartTime(encoder.getAcceleratedRelativeMovement(encoderMovement));
        rtcManager.printStartTime();
    }
    const uint32_t currentMillis = millis();
    if(currentMillis - previousTickTime > 1000) {
        rtcManager.printTimeLeftInCountdown();
        previousTickTime = currentMillis;
    }
    if (rtcManager.countdownElapsed()) {
        changeState(STATE_PROOFING);
    }
}

void stateProofingAct(int8_t encoderMovement) {
    (void) encoderMovement; // unused parameter...
    const uint32_t currentMillis = millis();
    if(currentMillis - previousTickTime > 1000) {
        rtcManager.printTimeProofing();
        const float currentTemperature = getTemperature();
        if(limitTemperature.proofingTemperatureTooLow(currentTemperature)) {
            // switch heater on
        }
        else if(limitTemperature.proofingTemperatureTooHigh(currentTemperature)) {
            //switch heater off
        }
        previousTickTime = currentMillis;
    }
}

void stateProofingInit() {
    Serial.println("Start heating!!!");
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(PIN_LED_PROOFING, HIGH);
    digitalWrite(PIN_LED_COLD, LOW);
    digitalWrite(PIN_LED_HOT, HIGH);
}
