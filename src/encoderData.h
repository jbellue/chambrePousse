#ifndef ENCODER_DATA_H
#define ENCODER_DATA_H

#include <Encoder.h>

int8_t getEncoderRelativeMovement(Encoder*);

int8_t getEncoderAcceleratedRelativeMovement(int8_t movement);

#endif