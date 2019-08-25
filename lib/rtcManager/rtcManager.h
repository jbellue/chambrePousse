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
        enum initReturn_t {
            success,
            lostPower,
            notFound
        };
        initReturn_t init();

        bool countdownElapsed();
        uint16_t getTempTime();
        uint16_t getStartTime();
        uint16_t getTimeLeftInCountdown();
        uint16_t getTimeProofing();
        uint16_t getRTCTime();

        void initSetTime();

        void initSetStartTime();

        void setTime(const int8_t diff);

        void setStartTime(const int8_t diff);

        void finishTimeSet();
        void finishStartTimeSet();
};

#endif