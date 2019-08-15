#include <limitTemperature.h>
#include <EEPROM.h>

void LimitTemperature::init() {
    _lowTemp = EEPROM.read(EEPROM_LOW_TEMP_ADDRESS);
    if ((uint8_t)_lowTemp == 255) {
        _lowTemp = DEFAULT_LOW_TEMP;
    }
    _proofingTemperature = EEPROM.read(EEPROM_PROOFING_TEMP_ADDRESS);
    if ((uint8_t)_proofingTemperature == 255) {
        _proofingTemperature = DEFAULT_PROOFING_TEMP;
    }
}
void LimitTemperature::setProofingTemperature(const int8_t difference) {
    _proofingTemperature += difference;
    Serial.println(_proofingTemperature);
}

int8_t LimitTemperature::getProofingTemperature() {
    return _proofingTemperature;
}

void LimitTemperature::storeProofingTemperature() {
    EEPROM.update(EEPROM_PROOFING_TEMP_ADDRESS, _proofingTemperature);
}

void LimitTemperature::setLowTemp(const int8_t difference) {
    _lowTemp += difference;
    Serial.println(_lowTemp);
}

int8_t LimitTemperature::getLowTemp() {
    return _proofingTemperature;
}

void LimitTemperature::storeLowTemp() {
    EEPROM.update(EEPROM_LOW_TEMP_ADDRESS, _lowTemp);
}

bool LimitTemperature::proofingTemperatureTooLow(const float t) {
    return (t < _proofingTemperature - ACCEPTABLE_DELTA_TEMPERATURE);
}

// TODO: figure out if the ACCEPTABLE_DELTA_TEMPERATURE is needed here,
// as the header might keep on warming a bit before stopping, and we
// probably don't want to overshoot too much.
// Consider having a LOW_DELTA and HIGH_DELTA
bool LimitTemperature::proofingTemperatureTooHigh(const float t) {
    return (t > _proofingTemperature + ACCEPTABLE_DELTA_TEMPERATURE);
}
