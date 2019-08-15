#include <stateMachine.h>

bool stateHasChanged = false;
State_t state = STATE_STARTUP;

StateMachine_t stateMachine[] = {
    {STATE_STARTUP,    NULL,               NULL},
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

void printStateToSerial() {
    switch(state) {
        case STATE_STARTUP:
            Serial.print("STARTUP");
            break;
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
    if (newState < State_t::NUM_STATE && newState != state) {
        Serial.print("Going from state ");
        printStateToSerial();
        state = newState;
        stateHasChanged = true;
        Serial.print(" to state ");
        printStateToSerial();
        Serial.println("");
        if ((*stateMachine[state].init)) {
            (*stateMachine[state].init)();
        }
    }
}

void stateMachineReact(const int8_t encoderMovement) {
    (*stateMachine[state].act)(encoderMovement);
}
