#include <rtcManager.h>

bool RTCManager::init() {
    if (!_rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    return !_rtc.lostPower();
}

bool RTCManager::countdownElapsed() {
        return _rtc.now() >= _startTime;
}
void RTCManager::printTempTime() {
    char timeBuffer[19];
    sprintf(timeBuffer, "New time: %d:%02d:00", _newTimeHours, _newTimeMinutes);
    Serial.println(timeBuffer);
}

void RTCManager::printStartTime() {
    char timeBuffer[25];
    sprintf(timeBuffer, "New start time: %d:%02d:%02d", _startTime.hour(), _startTime.minute(), _startTime.second());
    Serial.println(timeBuffer);
}

void RTCManager::printRTCTime() {
    DateTime now = _rtc.now();
    char timeBuffer[9];
    sprintf(timeBuffer, "%d:%02d:%02d", now.hour(), now.minute(), now.second());
    Serial.print(timeBuffer);
}

void RTCManager::printTimeLeftInCountdown() {
    TimeSpan timeLeft = _rtc.now() - _startTime;
    const int8_t tsHours =   timeLeft.hours()   < 0 ? -1 * timeLeft.hours()  :timeLeft.hours();
    const int8_t tsMinutes = timeLeft.minutes() < 0 ? -1 * timeLeft.minutes():timeLeft.minutes();
    const int8_t tsSeconds = timeLeft.seconds() < 0 ? -1 * timeLeft.seconds():timeLeft.seconds();
    char timeBuffer[20];
    sprintf(timeBuffer, "Time left: %d:%02d:%02d", tsHours, tsMinutes, tsSeconds);
    Serial.println(timeBuffer);
}

void RTCManager::printTimeProofing() {
    TimeSpan timeProofing = _rtc.now() - _startTime;
    char timeBuffer[24];
    sprintf(timeBuffer, "Time proofing: %d:%02d:%02d", timeProofing.hours(), timeProofing.minutes(), timeProofing.seconds());
    Serial.println(timeBuffer);
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
