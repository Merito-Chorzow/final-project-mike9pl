#ifndef FSM_H
#define FSM_H

#include <stdint.h>

// Stany FSM
enum class FsmState : uint8_t {
    INIT,
    IDLE,
    RUN,
    SAFE,
    FAULT,
};
// Fazy dla stanów (GŁÓWNYCH LEDÓW)
enum class IdlePhase : uint8_t { 
    RED, 
    YELLOW1, 
    GREEN, 
    YELLOW2 };
// Fazy dla stanów (LED)
enum class RunPhase : uint8_t { 
    SEC_RED, 
    SEC_GREEN 
};
// Inicjuje pętlę FSM.
void fsm_init();
// Aktualizuja stanu.
void fsm_set_state(FsmState s);
// Zwraca aktualny stan FSM.
FsmState fsm_get_state();

#endif
