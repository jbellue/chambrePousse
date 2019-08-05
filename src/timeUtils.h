#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <RTClib.h>

void initRTC(RTC_DS3231* rtc);

void printTempTime();

void printRTCTime(RTC_DS3231* rtc);
void initSetTime(RTC_DS3231* rtc);

void setTime(int8_t diff);

void finishTimeSet();

#endif