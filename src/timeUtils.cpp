#include <timeUtils.h>

int8_t newTimeMinutes = 0;
int8_t newTimeHours = 12;

DateTime startTime;

void initRTC(RTC_DS3231* rtc) {
    if (! rtc->begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    if (rtc->lostPower()) {
        Serial.println("RTC lost power, need to set the time!");
    }
}

void printTempTime() {
    char timeBuffer[6];
    sprintf(timeBuffer, "%d:%02d", newTimeHours, newTimeMinutes);
    Serial.println(timeBuffer);
}

void printStartTime() {
    char timeBuffer[6];
    sprintf(timeBuffer, "%d:%02d", startTime.hour(), startTime.minute());
    Serial.println(timeBuffer);
}

void printRTCTime(RTC_DS3231* rtc) {
    DateTime now = rtc->now();
    char timeBuffer[9];
    sprintf(timeBuffer, "%d:%02d:%02d", now.hour(), now.minute(), now.second());
    Serial.print(timeBuffer);
}

void printTimeLeft(RTC_DS3231* rtc) {
    TimeSpan left = rtc->now() - startTime;
    const int8_t tsHours =   left.hours()   < 0 ? -1 * left.hours()  :left.hours();
    const int8_t tsMinutes = left.minutes() < 0 ? -1 * left.minutes():left.minutes();
    const int8_t tsSeconds = left.seconds() < 0 ? -1 * left.seconds():left.seconds();
    char timeBuffer[10];
    sprintf(timeBuffer, "-%02d:%02d:%02d", tsHours, tsMinutes, tsSeconds);
    Serial.println(timeBuffer);
}

void initSetTime(RTC_DS3231* rtc) {
    const DateTime now = rtc->now();
    newTimeMinutes = now.minute();
    newTimeHours = now.hour();
}

void initSetStartTime(RTC_DS3231* rtc) {
    startTime = rtc->now();
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

void setStartTime(int8_t diff) {
    startTime = startTime + TimeSpan(diff * 60);
}


void finishTimeSet(RTC_DS3231* rtc) {
    // rtc.adjust(DateTime(2014, 1, 21, newTimeHours, newTimeMinutes, 0));
}

void finishStartTimeSet(RTC_DS3231* rtc) {
    DateTime now = rtc->now();
    if (startTime < now) {
        startTime = now;
    }
}

DateTime getStartTime() {
    return startTime;
}