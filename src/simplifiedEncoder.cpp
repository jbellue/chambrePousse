#include <simplifiedEncoder.h>

#define FAST_MS 75
#define VERY_FAST_MS 10

int8_t SimplifiedEncoder::getRelativeMovement() {
    int8_t ret = 0;
    const int32_t newPosition = read();

    // if the position has changed (and ignoring all the intermediate positions)
    if (newPosition != _oldPosition && (newPosition & 0b11) == 0b00 ) {
        if (newPosition < _oldPosition) {
            ret = -1;
        }
        else if (newPosition > _oldPosition) {
            ret = 1;
        }
        _oldPosition = newPosition;
    }
    return ret;
}

int8_t SimplifiedEncoder::getAcceleratedRelativeMovement(int8_t movement) {
    // if the position has changed
    if (movement) {
        const uint32_t now = millis();
        if (now - _lastRead < FAST_MS) {
            movement *= 5;
        }
        else if (now - _lastRead < VERY_FAST_MS) {
            movement *= 15;
        }
        _lastRead = now;
    }
    return movement;
}
