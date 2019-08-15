#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>

typedef enum {
    STATE_STARTUP,
    STATE_WAITING,
    STATE_TIME_UNSET,
    STATE_COUNTDOWN,
    STATE_PROOFING,
    NUM_STATE // a valid state will allways be < NUM_STATE
} State_t;

typedef struct {
    State_t State;
    void (*init)();
    void (*act)(const int8_t);
} StateMachine_t;

void stateWaitingInit();
void stateWaitingAct(const int8_t);
void stateTimeUnsetInit();
void stateTimeUnsetAct(const int8_t);
void stateCountdownInit();
void stateCountdownAct(const int8_t);
void stateProofingInit();
void stateProofingAct(const int8_t);

void printStateToSerial();
void changeState(State_t newState);

void stateMachineReact(const int8_t);

void runIfNewState(void(*)());
#endif