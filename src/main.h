#ifndef MAIN_H
#define MAIN_H

void setup();

float getTemperature();

void loop();

void displayTime(const uint16_t time, bool forceRefresh = false);
void displayTemperature(const float temp, bool showDecimal = true);

#endif
