#include <Arduino.h>
#include <Encoder.h>
#include <encoderData.h>
#include <stateMachine.h>
#include <RTClib.h>
#include <timeUtils.h>
#include <Button.h>
#include <limitTemperature.h>
#include <main.h>

RTC_DS3231 rtc;

// Use pins 2 and 3 because they're the only two with
// interrupt on the nano
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

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

    if (initRTC(&rtc)) {
        changeState(STATE_WAITING);
    }
    else {
        Serial.println("RTC lost power, need to set the time!");
        changeState(STATE_TIME_UNSET);
    }
}

void handleButtonSetLowTemp(int8_t* encoderMovement) {
    if(buttonSetLowTemp.read() == Button::PRESSED && *encoderMovement) {
        limitTemperature.setLowTemp(*encoderMovement);
        *encoderMovement = 0;
    }
    else if (buttonSetLowTemp.released()) {
        limitTemperature.storeLowTemp();
    }
}

void handleButtonSetProofingTemperature(int8_t* encoderMovement) {
    if(buttonSetProofingTemperature.read() == Button::PRESSED && *encoderMovement) {
        limitTemperature.setProofingTemperature(*encoderMovement);
        *encoderMovement = 0;
    }
    else if (buttonSetProofingTemperature.released()) {
        limitTemperature.storeProofingTemperature();
    }
}

void handleButtonSetTime(int8_t* encoderMovement) {
    if(buttonSetTime.pressed()) {
        initSetTime(&rtc);
    }
    else if(buttonSetTime.read() == Button::PRESSED && *encoderMovement) {
        setTime(getEncoderAcceleratedRelativeMovement(*encoderMovement));
        printTempTime();
        *encoderMovement = 0;
    }
    else if (buttonSetTime.released()) {
        finishTimeSet(&rtc);
    }
}

void loop() {
    int8_t encoderMovement = getEncoderRelativeMovement(&encoder);

    handleButtonSetLowTemp(&encoderMovement);
    handleButtonSetProofingTemperature(&encoderMovement);
    handleButtonSetTime(&encoderMovement);

    stateMachineReact(&encoderMovement);

    const uint32_t currentMillis = millis();
    if(currentMillis - previousMillis > 2000) {
        printRTCTime(&rtc);
        Serial.print(": ");
        printStateToSerial();
        Serial.println("");
        previousMillis = currentMillis;
    }
}

float getTemperature() {
    return rtc.getTemperature();
}

void blinkCountdownLED() {
    const uint32_t currentMillis = millis();
 
    if(currentMillis - lastBlinkingTime > 250) {
        lastBlinkingTime = currentMillis;   

        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
}

void stateWaitingInit() {
    initSetStartTime(&rtc);
    Serial.println("Setting the start time");
}

void stateWaitingAct(int8_t* encoderMovement) {
    if(*encoderMovement) {
        runIfNewState(stateWaitingInit);
        setStartTime(getEncoderAcceleratedRelativeMovement(*encoderMovement));
        *encoderMovement = 0;
        printStartTime();
    }
    else if (buttonEncoder.pressed()) {
        runIfNewState(stateWaitingInit);
        finishStartTimeSet(&rtc);
        changeState(STATE_COUNTDOWN);
    }
}

void stateTimeUnsetInit() {
    initSetTime(&rtc);
}
void stateTimeUnsetAct(int8_t* encoderMovement){
    if(*encoderMovement) {
        runIfNewState(stateTimeUnsetInit);
        setTime(getEncoderAcceleratedRelativeMovement(*encoderMovement));
        printTempTime();
        *encoderMovement = 0;
    }
    else if (buttonEncoder.pressed()) {
        runIfNewState(stateTimeUnsetInit);
        finishTimeSet(&rtc);
        changeState(STATE_WAITING);
    }
}


void stateCountdownInit() {
    digitalWrite(PIN_LED_COLD, HIGH);
}
void stateCountdownAct(int8_t* encoderMovement) {
    blinkCountdownLED();
    if(*encoderMovement) {
        setStartTime(getEncoderAcceleratedRelativeMovement(*encoderMovement));
        *encoderMovement = 0;
        printStartTime();
    }
    const uint32_t currentMillis = millis();
    if(currentMillis - previousTickTime > 1000) {
        printTimeLeftInCountdown(&rtc);
        previousTickTime = currentMillis;
    }
    if (rtc.now() >= getStartTime()) {
        changeState(STATE_PROOFING);
    }
}

void stateProofingAct(int8_t* encoderMovement) {
    (void) encoderMovement; // unused parameter...
    const uint32_t currentMillis = millis();
    if(currentMillis - previousTickTime > 1000) {
        printTimeProofing(&rtc);
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
