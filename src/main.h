#ifndef MAIN_H
#define MAIN_H
#include <Arduino.h>

typedef enum {
    STATE_WAITING,
    STATE_TIME_UNSET,
    STATE_COUNTDOWN,
    STATE_PROOFING,
    NUM_STATE // a valid state will allways be < NUM_STATE
} State_t;

typedef struct {
    State_t State;
    void (*init)();
    void (*act)(int8_t*);
} StateMachine_t;

void stateWaitingInit();
void stateWaitingAct(int8_t*);
void stateTimeUnsetInit();
void stateTimeUnsetAct(int8_t*);
void stateCountdownInit();
void stateCountdownAct(int8_t*);
void stateProofingInit();
void stateProofingAct(int8_t*);

StateMachine_t StateMachine[] = {
    { STATE_WAITING,   stateWaitingInit,    stateWaitingAct },
    { STATE_TIME_UNSET, stateTimeUnsetInit, stateTimeUnsetAct },
    { STATE_COUNTDOWN, stateCountdownInit,  stateCountdownAct },
    { STATE_PROOFING,  stateProofingInit,   stateProofingAct }
};

void printStateToSerial(State_t state);
void changeState(State_t newState);

void setup();

float getTemperature();

void setLowTemp(int8_t encoderMovement);

void finishLowTempSet();

void setProofingTemperature(int8_t encoderMovement);

void finishProofingTemperatureSet();

void blinkCountdownLED();

void loop();

#endif