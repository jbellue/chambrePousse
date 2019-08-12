#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>

enum State_t {
    STATE_WAITING,
    STATE_TIME_UNSET,
    STATE_COUNTDOWN,
    // STATE_TEMPERATURE_RAISING,
    STATE_PROOFING
};

void printStateToSerial(State_t state);
void changeState(State_t newState);

void setup();

float getTemperature();

void setLowTemp(int8_t encoderMovement);

void finishLowTempSet();

void setProofingTemperature(int8_t encoderMovement);

void finishProofingTemperatureSet();

void blinkCountdownLED();

void handleStateTimeUnset(int8_t* encoderMovement);

void handleStateWaiting(int8_t* encoderMovement);

void handleStateCountdown(int8_t* encoderMovement);

void handleStateProofing();

void loop();

#endif