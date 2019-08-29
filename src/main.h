#ifndef MAIN_H
#define MAIN_H

void setup();

float getTemperature();

void blinkCountdownLED();

void loop();

void displayTime(const uint16_t time, const uint8_t dots = 0b01000000);
void displayTemperature(const float temp, bool forceRefresh = false, bool showDecimal = true);

#endif
