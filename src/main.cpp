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

#define ENCODER_PIN_BTN 4
#define BTN_SET_TIME_PIN 5
#define BTN_SET_LOW_TEMP_PIN 6
#define BTN_SET_HIGH_TEMP_PIN 7
#define BTN_SET_START_TIME_PIN 8

#define EEPROM_LOW_TEMP_ADDRESS  0
#define DEFAULT_LOW_TEMP 4
#define EEPROM_HIGH_TEMP_ADDRESS 1
#define DEFAULT_HIGH_TEMP 24

Button buttonEncoder(ENCODER_PIN_BTN);
Button buttonSetTime(BTN_SET_TIME_PIN);
Button buttonSetLowTemp(BTN_SET_LOW_TEMP_PIN);
Button buttonSetHighTemp(BTN_SET_HIGH_TEMP_PIN);
Button buttonSetStartTime(BTN_SET_START_TIME_PIN);

bool timeIsSet = true;

uint32_t previousMillis = 0;

int8_t lowTemp = 4;
int8_t highTemp = 24;

enum {
    STATE_WAITING,
    STATE_KEEPING_COOL,
    STATE_TEMPERATURE_RAISING,
    STATE_PROOFING
} state;

void setup() {
    Serial.begin(9600);
    buttonEncoder.begin();
    buttonSetTime.begin();
    buttonSetStartTime.begin();
    buttonSetLowTemp.begin();
    buttonSetHighTemp.begin();

    lowTemp = EEPROM.read(EEPROM_LOW_TEMP_ADDRESS);
    if ((uint8_t)lowTemp == 255) {
        lowTemp = DEFAULT_LOW_TEMP;
    }
    initRTC(&rtc);

    state = STATE_WAITING;
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

void loop() {
    const int8_t encoderMovement = getEncoderRelativeMovement(&encoder);

    /* Button set time pressed */
    if(buttonSetTime.pressed()) {
        initSetTime(&rtc);
    }
    else if(buttonSetTime.read() == Button::PRESSED && encoderMovement) {
        setTime(getEncoderAcceleratedRelativeMovement(encoderMovement));
        printTempTime();
    }
    else if (buttonSetTime.released()) {
        finishTimeSet();
    }

    /* Button set low temperature pressed */
    if(buttonSetLowTemp.read() == Button::PRESSED && encoderMovement) {
        setLowTemp(encoderMovement);
    }
    else if (buttonSetLowTemp.released()) {
        finishLowTempSet();
    }

    /* Button set high temperature pressed */
    if(buttonSetHighTemp.read() == Button::PRESSED && encoderMovement) {
        setHighTemp(encoderMovement);
    }
    else if (buttonSetHighTemp.released()) {
        finishHighTempSet();
    }

    /* Button set start time pressed */
    if(buttonSetStartTime.pressed()) {
        initSetStartTime(&rtc);
    }
    else if(buttonSetStartTime.read() == Button::PRESSED && encoderMovement) {
        setStartTime(getEncoderAcceleratedRelativeMovement(encoderMovement));
        printStartTime();
    }
    else if (buttonSetStartTime.released()) {
        finishStartTimeSet(&rtc);
        state = STATE_KEEPING_COOL;
    }

    const uint32_t currentMillis = millis();

    if(currentMillis - previousMillis > 1000) {
        previousMillis = currentMillis;
        switch(state) {
            default:
            case STATE_WAITING:
                Serial.println("waiting...");
                break;
            case STATE_KEEPING_COOL:
                Serial.print("keeping cool...");
                if (getTemperature() > lowTemp) {
                    Serial.println("actively cooling");
                    // switch fridge on here
                }
                else {
                    Serial.println("already cool");
                    // switch fridge off here
                }
                if (rtc.now() >= getStartTime()) {
                    Serial.println("Start heating!!!");
                    state = STATE_TEMPERATURE_RAISING;
                }
                break;
            case STATE_TEMPERATURE_RAISING:
                Serial.println("raising temp...");
                // Slowly ramp up temperature
                if (getTemperature() >= highTemp) {
                    state = STATE_PROOFING;
                }
                break;
            case STATE_PROOFING:
                // keep temperature steady
                Serial.println("proofing...");
                break;
        }
        printRTCTime(&rtc);
    }
}
