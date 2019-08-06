#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <RTClib.h>

void initRTC(RTC_DS3231* rtc);

void printTempTime();
void printStartTime();


void printRTCTime(RTC_DS3231* rtc);
void initSetTime(RTC_DS3231* rtc);

void initSetStartTime(RTC_DS3231* rtc);

void setTime(int8_t diff);

void setStartTime(int8_t diff);

void finishTimeSet();
void finishStartTimeSet(RTC_DS3231* rtc);

DateTime getStartTime();

#endif