#pragma once
#include "sensorstate.h"

// How long do we debounce inputs for until we go into ERROR state?
#define SENSOR_ERROR_DEBOUNCE_PERIOD 50

class MonitoredSensor {
    protected:
        int pin_active;
        int pin_monitor;
        int when_active;     // Is the sensor active when the pin is LOW or when its HIGH?
        bool both_is_error;  // Is it an error when both pin_active and pin_monitor read active at the same time?

        sensor_state_t last_valid_state;
        unsigned long error_since;

        virtual int digital_read(int pin);

    public:
        MonitoredSensor(int pin_active, int pin_monitor, int when_active, bool both_is_error);
        sensor_state_t read(unsigned long millis);
};

#ifdef UNIT_TEST
class TestableMonitoredSensor : public MonitoredSensor {
    using MonitoredSensor::MonitoredSensor;

    protected:
        int pin_active_state;
        int pin_monitor_state;

        int digital_read(int pin);

    public:
        void set_pin_states(int pin_active_state, int pin_monitor_state);
};
#endif
