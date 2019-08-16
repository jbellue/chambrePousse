#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <RTClib.h>

class RTCManager {
    private:
        RTC_DS3231 _rtc;
        int8_t _newTimeMinutes = 0;
        int8_t _newTimeHours = 12;
        DateTime _startTime;
    public:
        bool init();

        bool countdownElapsed();
        void printTempTime();
        void printStartTime();
        void printTimeLeftInCountdown();
        void printTimeProofing();


        void printRTCTime();
        void initSetTime();

        void initSetStartTime();

        void setTime(const int8_t diff);

        void setStartTime(const int8_t diff);

        void finishTimeSet();
        void finishStartTimeSet();
};

#endif