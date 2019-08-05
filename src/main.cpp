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

#define EEPROM_LOW_TEMP_ADDRESS  0
#define DEFAULT_LOW_TEMP 4
#define EEPROM_HIGH_TEMP_ADDRESS 1
#define DEFAULT_HIGH_TEMP 24

Button buttonEncoder(ENCODER_PIN_BTN);
Button buttonSetTime(BTN_SET_TIME_PIN);
Button buttonSetLowTemp(BTN_SET_LOW_TEMP_PIN);

bool timeIsSet = true;

uint32_t previousMillis = 0;

int8_t lowTemp = 4;

void setup() {
    Serial.begin(9600);
    buttonEncoder.begin();
    buttonSetTime.begin();
    buttonSetLowTemp.begin();

    lowTemp = EEPROM.read(EEPROM_LOW_TEMP_ADDRESS);
    if ((uint8_t)lowTemp == 255) {
        lowTemp = DEFAULT_LOW_TEMP;
    }
    initRTC(&rtc);
}

void setLowTemp(int8_t encoderMovement) {
    lowTemp += encoderMovement;
    Serial.println(lowTemp);
}

void finishLowTempSet() {
    EEPROM.update(EEPROM_LOW_TEMP_ADDRESS, lowTemp);
}

void loop() {
    const int8_t encoderMovement = getEncoderRelativeMovement(&encoder);

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

    if(buttonSetLowTemp.read() == Button::PRESSED && encoderMovement) {
        setLowTemp(encoderMovement);
    }
    else if (buttonSetLowTemp.released()) {
        finishLowTempSet();
    }

    const uint32_t currentMillis = millis();

    if(currentMillis - previousMillis > 1000) {
        previousMillis = currentMillis;

        printRTCTime(&rtc);
    }
}
