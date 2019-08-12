#include <Arduino.h>
#include <Encoder.h>
#include <encoderData.h>
#include <RTClib.h>
#include <timeUtils.h>
#include <Button.h>
#include <EEPROM.h>
#include <main.h>

RTC_DS3231 rtc;

// Use pins 2 and 3 because they're the only two with
// interrupt on the nano
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);

// How many degrees away from the defined temperature are acceptable
#define ACCEPTABLE_DELTA_TEMPERATURE 1

#define PIN_BTN_ENCODER 4
#define PIN_BTN_SET_TIME 5
#define PIN_BTN_SET_LOW_TEMP 6
#define PIN_BTN_SET_HIGH_TEMP 7

#define PIN_LED_PROOFING 8
#define PIN_LED_COLD 10
#define PIN_LED_HOT 11

#define EEPROM_LOW_TEMP_ADDRESS  0
#define DEFAULT_LOW_TEMP 4
#define EEPROM_PROOFING_TEMP_ADDRESS 1
#define DEFAULT_HIGH_TEMP 24

Button buttonEncoder(PIN_BTN_ENCODER);
Button buttonSetTime(PIN_BTN_SET_TIME);
Button buttonSetLowTemp(PIN_BTN_SET_LOW_TEMP);
Button buttonSetProofingTemperature(PIN_BTN_SET_HIGH_TEMP);

uint32_t previousMillis = 0;
uint32_t previousTickTime = 0;
uint32_t lastBlinkingTime = 0;

int8_t lowTemp = 4;
int8_t proofingTemperature = 24;

State_t state;

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

    lowTemp = EEPROM.read(EEPROM_LOW_TEMP_ADDRESS);
    if ((uint8_t)lowTemp == 255) {
        lowTemp = DEFAULT_LOW_TEMP;
    }
    if (initRTC(&rtc)) {
        changeState(STATE_WAITING);
    }
    else {
        Serial.println("RTC lost power, need to set the time!");
        changeState(STATE_TIME_UNSET);
    }
}

void loop() {
    int8_t encoderMovement = getEncoderRelativeMovement(&encoder);

    /* Button set low temperature pressed */
    if(buttonSetLowTemp.read() == Button::PRESSED && encoderMovement) {
        setLowTemp(encoderMovement);
        encoderMovement = 0;
    }
    else if (buttonSetLowTemp.released()) {
        finishLowTempSet();
    }

    /* Button set high temperature pressed */
    if(buttonSetProofingTemperature.read() == Button::PRESSED && encoderMovement) {
        setProofingTemperature(encoderMovement);
        encoderMovement = 0;
    }
    else if (buttonSetProofingTemperature.released()) {
        finishProofingTemperatureSet();
    }

    (*StateMachine[state].act)(&encoderMovement);

    const uint32_t currentMillis = millis();
    if(currentMillis - previousMillis > 2000) {
        printRTCTime(&rtc);
        Serial.print(": ");
        printStateToSerial(state);
        Serial.println("");
        previousMillis = currentMillis;
    }
}

void printStateToSerial(State_t state) {
    switch(state) {
        case STATE_WAITING:
            Serial.print("WAITING");
            break;
        case STATE_TIME_UNSET:
            Serial.print("TIME_UNSET");
            break;
        case STATE_COUNTDOWN:
            Serial.print("COUNTDOWN");
            break;
        case STATE_PROOFING:
            Serial.print("PROOFING");
            break;
    }
}

void changeState(State_t newState) {
    if (newState < State_t::NUM_STATE) {
        Serial.print("Going from state ");
        printStateToSerial(state);
        state = newState;
        Serial.print(" to state ");
        printStateToSerial(state);
        Serial.println("");
        (*StateMachine[state].init)();
    }
}

float getTemperature() {
    return rtc.getTemperature();
}

void setLowTemp(int8_t encoderMovement) {
    lowTemp += encoderMovement;
    Serial.println(lowTemp);
}

void finishLowTempSet() {
    EEPROM.update(EEPROM_LOW_TEMP_ADDRESS, lowTemp);
}

void setProofingTemperature(int8_t encoderMovement) {
    proofingTemperature += encoderMovement;
    Serial.println(proofingTemperature);
}

void finishProofingTemperatureSet() {
    EEPROM.update(EEPROM_PROOFING_TEMP_ADDRESS, proofingTemperature);
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
        setStartTime(getEncoderAcceleratedRelativeMovement(*encoderMovement));
        *encoderMovement = 0;
        printStartTime();
    }
    else if (buttonEncoder.pressed()) {
        finishStartTimeSet(&rtc);
        changeState(STATE_COUNTDOWN);
    }
}

void stateTimeUnsetInit() {
    initSetTime(&rtc);
}
void stateTimeUnsetAct(int8_t* encoderMovement){
    if(*encoderMovement) {
        setTime(getEncoderAcceleratedRelativeMovement(*encoderMovement));
        printTempTime();
        *encoderMovement = 0;
    }
    else if (buttonEncoder.pressed()) {
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
        if (getTemperature() < proofingTemperature - ACCEPTABLE_DELTA_TEMPERATURE) {

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
