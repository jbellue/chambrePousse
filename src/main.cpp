#include <Arduino.h>
#include <Encoder.h>
#include "RTClib.h"
#include <Button.h>

RTC_DS3231 rtc;

// Use pins 2 and 3 because they're the only two with
// interrupt on the nano
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
#define ENCODER_FAST_MS 75
#define ENCODER_VERY_FAST_MS 10

#define ENCODER_PIN_BTN 4
#define BTN_SET_TIME_PIN 5
#define BTN_SET_LOW_TEMP_PIN 6

Button buttonEncoder(ENCODER_PIN_BTN);
Button buttonSetTime(BTN_SET_TIME_PIN);
Button buttonSetLowTemp(BTN_SET_LOW_TEMP_PIN);

bool timeIsSet = true;

int8_t newTimeMinutes = 0;
int8_t newTimeHours = 12;

uint32_t previousMillis = 0;

uint32_t lastEncoderRead = 0;
long oldEncoderPosition  = -999;

int8_t lowTemp = 4;

int8_t getEncoderRelativeMovement() {
    int8_t ret = 0;
    const int32_t newPosition = encoder.read();

    // if the position has changed (and ignoring all the intermediate positions)
    if (newPosition != oldEncoderPosition && (newPosition & 0b11) == 0b00 ) {
        if (newPosition < oldEncoderPosition) {
            ret = -1;
        }
        else if (newPosition > oldEncoderPosition) {
            ret = 1;
        }
        oldEncoderPosition = newPosition;
    }
    return ret;
}

int8_t getEncoderAcceleratedRelativeMovement(int8_t movement) {
    // if the position has changed
    if (movement) {
        const uint32_t now = millis();
        if (now - lastEncoderRead < ENCODER_FAST_MS) {
            movement *= 5;
        }
        else if (now - lastEncoderRead < ENCODER_VERY_FAST_MS) {
            movement *= 15;
        }
        lastEncoderRead = now;
    }
    return movement;
}

void setup() {
    Serial.begin(9600);
    buttonEncoder.begin();
    buttonSetTime.begin();
    buttonSetLowTemp.begin();

    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, need to set the time!");
        timeIsSet = false;
    }
}

void printTime() {
    char timeBuffer[6];
    sprintf(timeBuffer, "%d:%02d", newTimeHours, newTimeMinutes);
    Serial.println(timeBuffer);
}

void initSetTime() {
    const DateTime now = rtc.now();
    newTimeMinutes = now.minute();
    newTimeHours = now.hour();
}

void setTime(int8_t diff) {
    newTimeMinutes += diff;
    if (newTimeMinutes >= 60) {
        newTimeMinutes = 0;
        ++newTimeHours;
        if (newTimeHours >= 24) {
            newTimeHours = 0;
        }
    }
    else if (newTimeMinutes < 0) {
        newTimeMinutes = 59;
        --newTimeHours;
        if (newTimeHours < 0) {
            newTimeHours = 23;
        }
    }
}

void finishTimeSet() {
    // rtc.adjust(DateTime(2014, 1, 21, newTimeHours, newTimeMinutes, 0));
    timeIsSet = true;
}

void setLowTemp(int8_t encoderMovement) {
    lowTemp += encoderMovement;
    Serial.println(lowTemp);
}

void finishLowTempSet() {
    // Save to EEPROM
}

void loop() {
    const int8_t encoderMovement = getEncoderRelativeMovement();

    if(buttonSetTime.pressed()) {
        initSetTime();
    }
    else if(buttonSetTime.read() == Button::PRESSED && encoderMovement) {
        setTime(getEncoderAcceleratedRelativeMovement(encoderMovement));
        printTime();
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

        DateTime now = rtc.now();
        char timeBuffer[9];
        sprintf(timeBuffer, "%d:%02d:%02d", now.hour(), now.minute(), now.second());
        Serial.println(timeBuffer);
    }
}
