#ifndef MAIN_H
#define MAIN_H

void setup();

float getTemperature();

void setLowTemp(int8_t encoderMovement);

void finishLowTempSet();

void setProofingTemperature(int8_t encoderMovement);

void finishProofingTemperatureSet();

void blinkCountdownLED();

void loop();

#endif
