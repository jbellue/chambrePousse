#include <rtcManager.h>

RTCManager::initReturn_t RTCManager::init() {
    if (!_rtc.begin()) {
        return notFound;
    }

    if (_rtc.lostPower()) {
        return lostPower;
    }
    return success;
}

bool RTCManager::countdownElapsed() {
        return _rtc.now() >= _startTime;
}
uint16_t RTCManager::getTempTime() {
    return _newTimeHours * 100 + _newTimeMinutes;
}

uint16_t RTCManager::getStartTime() {
    return _startTime.hour() * 100 + _startTime.minute();
}

uint16_t RTCManager::getRTCTime() {
    DateTime now = _rtc.now();
    return now.hour() * 100 + now.minute();
}

uint16_t RTCManager::getTimeLeftInCountdown() {
    TimeSpan timeLeft = _rtc.now() - _startTime;
    return abs(timeLeft.hours()) * 100 + abs(timeLeft.minutes());
}

uint16_t RTCManager::getTimeProofing() {
    TimeSpan timeProofing = _rtc.now() - _startTime;
    return abs(timeProofing.hours()) * 100 + abs(timeProofing.minutes());
}

void RTCManager::initSetTime() {
    const DateTime now = _rtc.now();
    _newTimeMinutes = now.minute();
    _newTimeHours = now.hour();
}

void RTCManager::initSetStartTime() {
    _startTime = _rtc.now();
}

void RTCManager::setTime(const int8_t diff) {
    _newTimeMinutes += diff;
    if (_newTimeMinutes >= 60) {
        _newTimeMinutes = 0;
        ++_newTimeHours;
        if (_newTimeHours >= 24) {
            _newTimeHours = 0;
        }
    }
    else if (_newTimeMinutes < 0) {
        _newTimeMinutes = 59;
        --_newTimeHours;
        if (_newTimeHours < 0) {
            _newTimeHours = 23;
        }
    }
}

void RTCManager::setStartTime(const int8_t diff) {
    _startTime = _startTime + TimeSpan(diff * 60);
}


void RTCManager::finishTimeSet() {
    // rtc.adjust(DateTime(2014, 1, 21, newTimeHours, newTimeMinutes, 0));
}

void RTCManager::finishStartTimeSet() {
    DateTime now = _rtc.now();
    if (_startTime < now) {
        _startTime = now;
    }
}
