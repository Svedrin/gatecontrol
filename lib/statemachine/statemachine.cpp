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

    if (
        esp_state->sensor_gate_up       == SENSOR_ERROR ||
        esp_state->sensor_gate_down     == SENSOR_ERROR ||
        esp_state->sensor_light_barrier == SENSOR_ERROR
    ) {
        this->current_state = GATE_ERROR;
    }

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
            if( esp_state->sensor_light_barrier == SENSOR_ACTIVE ){
                this->current_state = GATE_BLOCKED;
            }
            else if( esp_state->sensor_gate_up == SENSOR_CLEAR ){
                this->current_state = GATE_UNKNOWN;
            }
            break;

        case GATE_UNKNOWN:
            if( esp_state->sensor_gate_down == SENSOR_ACTIVE ){
                this->current_state = GATE_CLOSED;
            }
            else if( esp_state->sensor_gate_up == SENSOR_ACTIVE ){
                this->current_state = GATE_OPEN;
            }
            break;

        case GATE_CLOSED:
            if( esp_state->sensor_gate_down == SENSOR_CLEAR ){
                this->current_state = GATE_UNKNOWN;
            }
            break;

        case GATE_BLOCKED:
            if( esp_state->sensor_light_barrier == SENSOR_CLEAR ){
                this->current_state = GATE_OPEN;
            }
            break;

        case GATE_CLOSE_PREPARE:
            if( esp_state->sensor_light_barrier == SENSOR_ACTIVE ){
                this->current_state = GATE_BLOCKED;
            }
            else if( esp_state->sensor_gate_up == SENSOR_CLEAR ){
                this->current_state = GATE_UNKNOWN;
            }
            else if (this->received_commit_at == 0) {
                // No commit yet, see if that means anything
                if (esp_state->millis > this->received_close_at + 10100) {
                    // Commit timed out
                    this->current_state = GATE_OPEN;
                }
            }
            else if (
                abs(received_close_at + 10000 - received_commit_at) < 100
            ) {
                // Commit came, and on time
                next_step.trigger = true;
                this->current_state = GATE_CLOSE_TRIGGERED;
            }
            else {
                // Commit came, but time was off
                this->current_state = GATE_OPEN;
            }
            break;

        case GATE_CLOSE_TRIGGERED:
            if( esp_state->sensor_gate_up == SENSOR_CLEAR ){
                this->current_state = GATE_UNKNOWN;
            }
            else if (
                esp_state->millis > this->received_commit_at + 5000
            ) {
                // 5 seconds should more than suffice, something's wrong
                this->current_state = GATE_ERROR;
            }
            break;

        case GATE_ERROR:
            // No way out, gotta hit reset
            break;
    }
    next_step.current_state = this->current_state;
    return next_step;
}

cmd_result_t StateMachine::cmd_close(unsigned long received_at) {
    if(this->current_state == GATE_OPEN){
        this->received_close_at  = received_at;
        this->received_commit_at = 0;
        this->current_state      = GATE_CLOSE_PREPARE;
        return COMMAND_ACCEPTED;
    }
    return COMMAND_IGNORED;
}

cmd_result_t StateMachine::cmd_commit(unsigned long received_at) {
    if(this->current_state == GATE_CLOSE_PREPARE){
        this->received_commit_at = received_at;
        return COMMAND_ACCEPTED;
    }
    return COMMAND_IGNORED;
}
