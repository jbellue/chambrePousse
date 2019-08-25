#include <stateMachine.h>
#include <debug_macros.h>

bool stateHasChanged = false;
State_t state = STATE_STARTUP;

StateMachine_t stateMachine[] = {
    {STATE_STARTUP,    NULL},
    {STATE_WAITING,    stateWaitingAct},
    {STATE_TIME_UNSET, stateTimeUnsetAct},
    {STATE_COUNTDOWN,  stateCountdownAct},
    {STATE_PROOFING,   stateProofingAct}
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
            DebugPrint("STARTUP");
            break;
        case STATE_WAITING:
            DebugPrint("WAITING");
            break;
        case STATE_TIME_UNSET:
            DebugPrint("TIME_UNSET");
            break;
        case STATE_COUNTDOWN:
            DebugPrint("COUNTDOWN");
            break;
        case STATE_PROOFING:
            DebugPrint("PROOFING");
            break;
        default:
            break;
    }
}

void changeState(State_t newState) {
    if (newState < State_t::NUM_STATE && newState != state) {
        DebugPrintFull("Going from state ");
        printStateToSerial();
        state = newState;
        stateHasChanged = true;
        DebugPrint(" to state ");
        printStateToSerial();
        DebugPrintln("");
    }
}

void stateMachineReact(const int8_t encoderMovement) {
    (*stateMachine[state].act)(encoderMovement);
}
