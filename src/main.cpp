#include <Arduino.h>
#include <Encoder.h>
#include <encoderData.h>
#include <RTClib.h>
#include <timeUtils.h>
#include <Button.h>
#include <EEPROM.h>

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

#define EEPROM_LOW_TEMP_ADDRESS  0
#define DEFAULT_LOW_TEMP 4
#define EEPROM_HIGH_TEMP_ADDRESS 1
#define DEFAULT_HIGH_TEMP 24

Button buttonEncoder(PIN_BTN_ENCODER);
Button buttonSetTime(PIN_BTN_SET_TIME);
Button buttonSetLowTemp(PIN_BTN_SET_LOW_TEMP);
Button buttonSetHighTemp(PIN_BTN_SET_HIGH_TEMP);

bool timeIsSet = true;

uint32_t previousMillis = 0;
uint32_t lastBlinkingTime = 0;

int8_t lowTemp = 4;
int8_t highTemp = 24;

enum State_t {
    STATE_WAITING,
    STATE_TIME_UNSET,
    STATE_COUNTDOWN,
    // STATE_TEMPERATURE_RAISING,
    STATE_PROOFING
} state;
bool stateIsNew = true;

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
    Serial.print("Going from state ");
    printStateToSerial(state);
    state = newState;
    Serial.print(" to state ");
    printStateToSerial(state);
    Serial.println("");
    stateIsNew = true;
}

void setup() {
    Serial.begin(9600);
    buttonEncoder.begin();
    buttonSetTime.begin();
    buttonSetLowTemp.begin();
    buttonSetHighTemp.begin();

    pinMode(PIN_LED_PROOFING, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

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

void setHighTemp(int8_t encoderMovement) {
    highTemp += encoderMovement;
    Serial.println(highTemp);
}

void finishHighTempSet() {
    EEPROM.update(EEPROM_HIGH_TEMP_ADDRESS, highTemp);
}

void blinkCountdownLED() {
    const uint32_t currentMillis = millis();
 
    if(currentMillis - lastBlinkingTime > 250) {
        lastBlinkingTime = currentMillis;   

        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
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
    if(buttonSetHighTemp.read() == Button::PRESSED && encoderMovement) {
        setHighTemp(encoderMovement);
        encoderMovement = 0;
    }
    else if (buttonSetHighTemp.released()) {
        finishHighTempSet();
    }

    switch(state) {
        case STATE_TIME_UNSET:
            if(encoderMovement) {
                if(stateIsNew) {
                    initSetTime(&rtc);
                    stateIsNew = false;
                }
                setTime(getEncoderAcceleratedRelativeMovement(encoderMovement));
                printTempTime();
                encoderMovement = 0;
            }
            else if (buttonEncoder.pressed()) {
                if(stateIsNew) {
                    initSetTime(&rtc);
                    stateIsNew = false;
                }
                finishTimeSet(&rtc);
                changeState(STATE_WAITING);
            }
            break;
        default:
        case STATE_WAITING:
            if(encoderMovement) {
                if(stateIsNew) {
                    initSetStartTime(&rtc);
                    Serial.println("Setting the start time");
                    stateIsNew = false;
                }
                setStartTime(getEncoderAcceleratedRelativeMovement(encoderMovement));
                encoderMovement = 0;
                printStartTime();
            }
            else if (buttonEncoder.pressed()) {
                if(stateIsNew) {
                    initSetStartTime(&rtc);
                    stateIsNew = false;
                }
                finishStartTimeSet(&rtc);
                changeState(STATE_COUNTDOWN);
            }
            break;
        case STATE_COUNTDOWN:
            blinkCountdownLED();
            if (rtc.now() >= getStartTime()) {
                Serial.println("Start heating!!!");
                digitalWrite(LED_BUILTIN, LOW);
                digitalWrite(PIN_LED_PROOFING, HIGH);
                changeState(STATE_PROOFING);
            }
            break;
        case STATE_PROOFING:
            // keep temperature steady
            break;
    }

    const uint32_t currentMillis = millis();
    if(currentMillis - previousMillis > 2000) {
        printRTCTime(&rtc);
        Serial.print(": ");
        printStateToSerial(state);
        Serial.println("");
        previousMillis = currentMillis;
    }
}
