#ifndef ENCODER_DATA_H
#define ENCODER_DATA_H

#include <Encoder.h>

class SimplifiedEncoder:Encoder {
    private:
        uint32_t _lastRead;
        long _oldPosition;
    public:
        SimplifiedEncoder(uint8_t pinA, uint8_t pinB):
            Encoder(pinA, pinB), 
            _lastRead(0),
            _oldPosition(0) {}
        int8_t getRelativeMovement();
        int8_t getAcceleratedRelativeMovement(int8_t movement);
};

#endif