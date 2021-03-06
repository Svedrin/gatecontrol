#pragma GCC diagnostic push  // save compiler settings
#pragma GCC diagnostic error "-Wall"

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
    this->entered_owait_at   = 0;
    this->received_close_at  = 0;
    this->received_commit_at = 0;
    this->autoclose_enabled_at = 0;
    this->autoclose_timer_started_at = 0;
    this->triggered_at = 0;
}

step_t StateMachine::step(esp_state_t *esp_state) {
    step_t next_step;
    next_step.trigger = false;
    next_step.previous_state = this->current_state;
    next_step.autoclose_state =
        (this->autoclose_enabled_at ? AUTOCLOSE_ON : AUTOCLOSE_OFF);

    if (
        esp_state->sensor_gate_up       == SENSOR_ERROR ||
        esp_state->sensor_gate_down     == SENSOR_ERROR ||
        esp_state->sensor_light_barrier == SENSOR_ERROR ||
        esp_state->button_autoclose     == SENSOR_ERROR ||
        (
            esp_state->sensor_gate_up   == SENSOR_ACTIVE &&
            esp_state->sensor_gate_down == SENSOR_ACTIVE
        )
    ) {
        this->current_state = GATE_ERROR;
    }

    switch(this->current_state){
        case GATE_INIT:
            if( esp_state->sensor_gate_down == SENSOR_ACTIVE ){
                this->current_state = GATE_CLOSED;
            }
            else if( esp_state->sensor_gate_up == SENSOR_ACTIVE ){
                this->current_state = GATE_OWAIT;
                this->entered_owait_at = esp_state->millis;
            }
            else {
                this->current_state = GATE_UNKNOWN;
            }
            break;

        case GATE_OWAIT:
            if( esp_state->sensor_gate_up == SENSOR_ACTIVE ){
                if( esp_state->millis >= this->entered_owait_at + OWAIT_TIMEOUT ){
                    this->current_state = GATE_OPEN;
                    this->entered_owait_at = 0;
                }
            }
            else if( esp_state->sensor_gate_up == SENSOR_CLEAR ){
                this->current_state = GATE_UNKNOWN;
                this->entered_owait_at = 0;
            }
            break;

        case GATE_OPEN:
            if( esp_state->sensor_light_barrier == SENSOR_ACTIVE ){
                this->current_state = GATE_BLOCKED;
            }
            else if( esp_state->sensor_gate_up == SENSOR_CLEAR ){
                this->current_state = GATE_UNKNOWN;
            }
            else if (this->autoclose_enabled_at) {
                if (esp_state->millis > this->autoclose_enabled_at + AUTOCLOSE_TIMEOUT) {
                    this->autoclose_enabled_at = 0;
                    next_step.autoclose_state = AUTOCLOSE_RESET;
                }
            }
            else if (esp_state->button_autoclose == SENSOR_ACTIVE) {
                this->autoclose_enabled_at = esp_state->millis;
                next_step.autoclose_state = AUTOCLOSE_ENABLED;
            }
            break;

        case GATE_UNKNOWN:
            if( esp_state->sensor_gate_down == SENSOR_ACTIVE ){
                this->current_state = GATE_CLOSED;
                this->autoclose_enabled_at = 0;
                next_step.autoclose_state = AUTOCLOSE_OFF;
            }
            else if( esp_state->sensor_gate_up == SENSOR_ACTIVE ){
                this->current_state = GATE_OWAIT;
                this->entered_owait_at = esp_state->millis;
            }
            else if (esp_state->button_autoclose == SENSOR_ACTIVE) {
                this->autoclose_enabled_at = esp_state->millis;
                next_step.autoclose_state = AUTOCLOSE_ENABLED;
            }
            break;

        case GATE_CLOSED:
            if( esp_state->sensor_gate_down == SENSOR_CLEAR ){
                this->current_state = GATE_UNKNOWN;
            }
            else if (esp_state->button_autoclose == SENSOR_ACTIVE) {
                this->autoclose_enabled_at = esp_state->millis;
                this->current_state = GATE_OPEN_TRIGGERED;
                this->triggered_at  = esp_state->millis;
                next_step.autoclose_state = AUTOCLOSE_ENABLED;
                next_step.trigger = true;
            }
            else if (
                this->autoclose_enabled_at &&
                esp_state->millis > this->triggered_at + GATE_ERROR_TIMEOUT
            ) {
                // 5 seconds should more than suffice, something's wrong
                this->current_state = GATE_ERROR;
            }
            break;

        case GATE_BLOCKED:
            if( esp_state->sensor_light_barrier == SENSOR_CLEAR ){
                if (this->autoclose_enabled_at) {
                    this->autoclose_timer_started_at = esp_state->millis;
                    this->current_state = GATE_CLOSE_AUTO;
                    next_step.autoclose_state = AUTOCLOSE_PENDING;
                }
                else {
                    this->current_state = GATE_OPEN;
                }
            }
            else if( esp_state->sensor_gate_up == SENSOR_CLEAR ){
                this->current_state = GATE_UNKNOWN;
            }
            break;

        case GATE_CLOSE_AUTO:
            if( esp_state->sensor_light_barrier == SENSOR_ACTIVE ){
                this->current_state = GATE_BLOCKED;
                next_step.autoclose_state = AUTOCLOSE_RESET;
            }
            else if( esp_state->sensor_gate_up == SENSOR_CLEAR ){
                this->current_state = GATE_UNKNOWN;
                // Let's take this as a sign they changed their mind about autoclose
                this->autoclose_enabled_at = 0;
                next_step.autoclose_state = AUTOCLOSE_RESET;
            }
            else if (esp_state->millis > this->autoclose_timer_started_at + AUTOCLOSE_WAIT_PERIOD) {
                this->current_state = GATE_OPEN;
                this->autoclose_enabled_at = 0;
                next_step.autoclose_state = AUTOCLOSE_TRIGGERED;
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
                if (esp_state->millis > this->received_close_at + CLOSE_TILL_COMMIT + 100) {
                    // Commit timed out
                    this->current_state = GATE_OPEN;
                }
            }
            else if (
                abs(received_close_at + CLOSE_TILL_COMMIT - received_commit_at) < 100
            ) {
                // Commit came, and on time
                next_step.trigger = true;
                this->current_state = GATE_CLOSE_TRIGGERED;
                this->triggered_at  = esp_state->millis;
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
            else if (esp_state->millis > this->triggered_at + GATE_ERROR_TIMEOUT) {
                // 5 seconds should more than suffice, something's wrong
                this->current_state = GATE_ERROR;
            }
            break;

        case GATE_OPEN_TRIGGERED:
            if (esp_state->sensor_gate_down == SENSOR_CLEAR) {
                this->current_state = GATE_UNKNOWN;
            }
            else if (esp_state->millis > this->triggered_at + GATE_ERROR_TIMEOUT) {
                // 5 seconds should more than suffice, something's wrong
                this->current_state = GATE_ERROR;
            }
            else if (esp_state->button_autoclose == SENSOR_CLEAR) {
                this->current_state = GATE_CLOSED;
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
    // Only accept MQTT commands while autoclose is disabled.
    // This ensures that a person waiting for autoclose to kick in
    // cannot be surprised by someone else just triggering
    // the gate remotely via MQTT.
    if(
        this->current_state == GATE_OPEN &&
        this->autoclose_enabled_at == 0
    ){
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

#pragma GCC diagnostic pop // Restore compiler settings
