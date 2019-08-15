#ifndef LIMIT_TEMPERATURE_H
#define LIMIT_TEMPERATURE_H

#include <Arduino.h>

#define EEPROM_LOW_TEMP_ADDRESS  0
#define DEFAULT_LOW_TEMP 4
#define EEPROM_PROOFING_TEMP_ADDRESS 1
#define DEFAULT_PROOFING_TEMP 24

// How many degrees away from the defined temperature are acceptable
#define ACCEPTABLE_DELTA_TEMPERATURE 1

class LimitTemperature {
    public:
        void init();
        void setProofingTemperature(const int8_t difference);
        int8_t getProofingTemperature();
        void storeProofingTemperature();

        void setLowTemp(const int8_t difference);
        int8_t getLowTemp();
        void storeLowTemp();

        bool proofingTemperatureTooLow(const float t);
        bool proofingTemperatureTooHigh(const float t);
    private:
        int8_t _lowTemp;
        int8_t _proofingTemperature;
};

#endif