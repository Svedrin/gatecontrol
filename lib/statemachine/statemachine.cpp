#ifndef UNIT_TEST
#include <Arduino.h>
#else
unsigned long abs(long long a) {
    if (a < 0)
        return -a;
    return a;
}
#endif

#include "statemachine.h"

StateMachine::StateMachine() {
    this->current_state = GATE_INIT;
    this->received_close_at = 0;
    this->received_commit_at = 0;
}

step_t StateMachine::step(esp_state_t *esp_state) {
    step_t next_step;
    next_step.trigger = false;
    next_step.previous_state = this->current_state;
    switch(this->current_state){
        default:
        case GATE_INIT:
            if( esp_state->sensor_gate_down == SENSOR_ACTIVE ){
                this->current_state = GATE_CLOSED;
            }
            else if( esp_state->sensor_gate_up == SENSOR_ACTIVE ){
                this->current_state = GATE_OPEN;
            }
            else {
                this->current_state = GATE_UNKNOWN;
            }
            break;

        case GATE_OPEN:
            if( esp_state->sensor_lb_blocked == SENSOR_ACTIVE ){
                if(this->received_close_at != 0){
                    this->received_close_at  = 0;
                    this->received_commit_at = 0;
                }
            }
            else if( esp_state->sensor_lb_clear == SENSOR_CLEAR ){
                this->current_state = GATE_ERROR;
            }
            else if( esp_state->sensor_gate_up != SENSOR_ACTIVE ){
                this->current_state = GATE_UNKNOWN;
            }
            else if(this->received_close_at  != 0 ){
                if( this->received_commit_at != 0 ){
                    if(abs(received_close_at + 10000 - received_commit_at) < 100){
                        next_step.trigger = true;
                    }
                    received_close_at   = 0;
                    received_commit_at  = 0;
                }
            }
            break;

        case GATE_UNKNOWN:
            this->received_close_at   = 0;
            this->received_commit_at  = 0;
            if( esp_state->sensor_gate_down == SENSOR_ACTIVE ){
                this->current_state = GATE_CLOSED;
            }
            else if( esp_state->sensor_gate_up == SENSOR_ACTIVE ){
                this->current_state = GATE_OPEN;
            }
            break;

        case GATE_CLOSED:
            this->received_close_at   = 0;
            this->received_commit_at  = 0;
            if( esp_state->sensor_gate_down != SENSOR_ACTIVE ){
                this->current_state = GATE_UNKNOWN;
            }
            break;

    }
    next_step.current_state = this->current_state;
    return next_step;
}

cmd_result_t StateMachine::cmd_close(unsigned long received_at) {
    if(this->current_state == GATE_OPEN && this->received_close_at == 0){
        received_close_at = received_at;
        return COMMAND_ACCEPTED;
    }
    return COMMAND_IGNORED;
}

cmd_result_t StateMachine::cmd_commit(unsigned long received_at) {
    if(this->current_state == GATE_OPEN && this->received_commit_at == 0){
        received_commit_at = received_at;
        return COMMAND_ACCEPTED;
    }
    return COMMAND_IGNORED;
}
