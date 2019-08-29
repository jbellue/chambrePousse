#ifndef MAIN_H
#define MAIN_H

void setup();

float getTemperature();

void loop();

void displayTime(const uint16_t time);
void displayTemperature(const float temp, bool forceRefresh = false, bool showDecimal = true);

#endif
