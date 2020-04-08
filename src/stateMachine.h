#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>

/* Forward declare the functions used in the state machine */
void stateWaitingAct(const int8_t);
void stateTimeUnsetAct(const int8_t);
void stateCountdownAct(const int8_t);
void stateProofingAct(const int8_t);

class StateMachine {
    public:
        typedef enum {
            STATE_STARTUP,
            STATE_WAITING,
            STATE_TIME_UNSET,
            STATE_COUNTDOWN,
            STATE_PROOFING,
            NUM_STATE // a valid state will always be < NUM_STATE
        } State_t;
        StateMachine(): _stateHasChanged(false),
                        _state(STATE_STARTUP),
                        _stateMachine({
                            {STATE_STARTUP,    NULL},
                            {STATE_WAITING,    stateWaitingAct},
                            {STATE_TIME_UNSET, stateTimeUnsetAct},
                            {STATE_COUNTDOWN,  stateCountdownAct},
                            {STATE_PROOFING,   stateProofingAct}
                        }) {};
        
        void printStateToSerial();
        void changeState(const State_t newState);
        void stateMachineReact(const int8_t);
        void runIfNewState(void(*)());
        
    private:
        /* Makes a State_t correspond to a function to execute */
        typedef struct {
            State_t State;
            void (*act)(const int8_t);
        } StateMachine_t;
        bool _stateHasChanged;
        State_t _state;
        StateMachine_t _stateMachine[5];
};

#endif