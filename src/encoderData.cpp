#include <encoderData.h>


#define ENCODER_FAST_MS 75
#define ENCODER_VERY_FAST_MS 10

uint32_t lastEncoderRead = 0;
long oldEncoderPosition  = -999;

int8_t getEncoderRelativeMovement(Encoder* encoder) {
    int8_t ret = 0;
    const int32_t newPosition = encoder->read();

    // if the position has changed (and ignoring all the intermediate positions)
    if (newPosition != oldEncoderPosition && (newPosition & 0b11) == 0b00 ) {
        if (newPosition < oldEncoderPosition) {
            ret = -1;
        }
        else if (newPosition > oldEncoderPosition) {
            ret = 1;
        }
        oldEncoderPosition = newPosition;
    }
    return ret;
}

int8_t getEncoderAcceleratedRelativeMovement(int8_t movement) {
    // if the position has changed
    if (movement) {
        const uint32_t now = millis();
        if (now - lastEncoderRead < ENCODER_FAST_MS) {
            movement *= 5;
        }
        else if (now - lastEncoderRead < ENCODER_VERY_FAST_MS) {
            movement *= 15;
        }
        lastEncoderRead = now;
    }
    return movement;
}