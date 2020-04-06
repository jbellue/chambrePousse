#include <LEDcontroller.h>

const uint8_t brightnessLookupTable[8] = {0, 5, 14, 33, 64, 109, 172, 255};

void LED::init() {
    pinMode(_pin, OUTPUT);
}
void LED::on() {
    _on = true;
    analogWrite(_pin, _brightness);
}
void LED::off() {
    _on = false;
    analogWrite(_pin, 0);
}
void LED::toggle() {
    _on = !_on;
    analogWrite(_pin, _on?_brightness : 0);
}

void LED::setBrightness(const uint8_t b) {
    if (b < 8) {
        _brightness = brightnessLookupTable[b];
        if (_on) {
            analogWrite(_pin, _brightness);
        }
    }
    else {
        return;
    }
}