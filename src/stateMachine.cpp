#include <stateMachine.h>
#include <debug_macros.h>

void StateMachine::runIfNewState(void(*functionToRun)()) {
    if (_stateHasChanged) {
        if(functionToRun) {
            (*functionToRun)();
        }
        _stateHasChanged = false;
    }
}

void StateMachine::printStateToSerial() {
    switch(_state) {
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

void StateMachine::changeState(State_t newState) {
    if (newState < State_t::NUM_STATE && newState != _state) {
        DebugPrintFull("Going from state ");
        printStateToSerial();
        _state = newState;
        _stateHasChanged = true;
        DebugPrint(" to state ");
        printStateToSerial();
        DebugPrintln("");
    }
}

void StateMachine::stateMachineReact(const int8_t encoderMovement) {
    (*_stateMachine[_state].act)(encoderMovement);
}
