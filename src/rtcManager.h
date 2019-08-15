#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>

class RTCManager {
    private:
        RTC_DS3231 _rtc;
        int8_t _newTimeMinutes = 0;
        int8_t _newTimeHours = 12;
        DateTime _startTime;
    public:
        bool init();

        DateTime now() { return _rtc.now(); };
        void printTempTime();
        void printStartTime();
        void printTimeLeftInCountdown();
        void printTimeProofing();


        void printRTCTime();
        void initSetTime();

        void initSetStartTime();

        void setTime(int8_t diff);

        void setStartTime(int8_t diff);

        void finishTimeSet();
        void finishStartTimeSet();

        DateTime getStartTime();
};


#endif