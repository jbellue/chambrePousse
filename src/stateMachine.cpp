#include <stateMachine.h>

bool stateHasChanged = false;
extern State_t state;

StateMachine_t StateMachine[] = {
    {STATE_WAITING,    NULL,               stateWaitingAct},
    {STATE_TIME_UNSET, NULL,               stateTimeUnsetAct},
    {STATE_COUNTDOWN,  stateCountdownInit, stateCountdownAct},
    {STATE_PROOFING,   stateProofingInit,  stateProofingAct}
};

void runIfNewState(void(*functionToRun)()) {
    if (stateHasChanged) {
        if(functionToRun) {
            (*functionToRun)();
        }
        stateHasChanged = false;
    }
}

void printStateToSerial(State_t state) {
    switch(state) {
        case STATE_WAITING:
            Serial.print("WAITING");
            break;
        case STATE_TIME_UNSET:
            Serial.print("TIME_UNSET");
            break;
        case STATE_COUNTDOWN:
            Serial.print("COUNTDOWN");
            break;
        case STATE_PROOFING:
            Serial.print("PROOFING");
            break;
        default:
            break;
    }
}

void changeState(State_t newState) {
    if (newState < State_t::NUM_STATE) {
        Serial.print("Going from state ");
        printStateToSerial(state);
        state = newState;
        stateHasChanged = true;
        Serial.print(" to state ");
        printStateToSerial(state);
        Serial.println("");
        if ((*StateMachine[state].init)) {
            (*StateMachine[state].init)();
        }
    }
}