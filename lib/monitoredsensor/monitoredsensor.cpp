#include "monitoredsensor.h"

MonitoredSensor::MonitoredSensor(int pin_active, int pin_monitor, int when_active) {
    this->pin_active   = pin_active;
    this->pin_monitor  = pin_monitor;
    this->when_active  = when_active;
    this->last_valid_state = SENSOR_INIT;
    this->error_since  = 0;
}

sensor_state_t MonitoredSensor::read(unsigned long millis) {
    if (this->digital_read(this->pin_active) == this->when_active) {
        // Valid state, active pin is signalling
        this->last_valid_state = SENSOR_ACTIVE;
        this->error_since = 0;
        return SENSOR_ACTIVE;
    } else if (this->digital_read(this->pin_monitor) == this->when_active) {
        // Valid state, active pin is not signalling but monitor pin _is_
        this->last_valid_state = SENSOR_CLEAR;
        this->error_since = 0;
        return SENSOR_CLEAR;
    } else if (this->error_since == 0) {
        // Both are gone. let's debounce this for 50ms
        this->error_since = millis;
        return this->last_valid_state;
    } else if (millis < this->error_since + 50) {
        // debounce period not yet expired
        return this->last_valid_state;
    } else {
        // Valid state again: error but debounced
        this->last_valid_state = SENSOR_ERROR;
        return SENSOR_ERROR;
    }
}

#ifndef UNIT_TEST
#include <Arduino.h>

int MonitoredSensor::digital_read(int pin) {
    return digitalRead(pin);
}

#else

int MonitoredSensor::digital_read(int pin) {
    return 0;
}

void TestableMonitoredSensor::set_pin_states(int pin_active_state, int pin_monitor_state) {
    this->pin_active_state = pin_active_state;
    this->pin_monitor_state = pin_monitor_state;
}

int TestableMonitoredSensor::digital_read(int pin) {
    if (pin == this->pin_active) {
        return this->pin_active_state;
    }
    return this->pin_monitor_state;
}
#endif
