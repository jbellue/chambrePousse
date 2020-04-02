#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <Arduino.h>

class LED {
    public:
        LED(const uint8_t pin, const uint8_t brightness = 255):
            _pin(pin), _brightness(brightness), _on(false) {};
        void init();
        void on();
        void off();
        void toggle();
        void setBrightness(const uint8_t b);

    private:
        uint8_t _pin;
        uint8_t _brightness;
        bool _on;
};

#endif