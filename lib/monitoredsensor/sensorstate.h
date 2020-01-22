#pragma once
typedef enum {
    SENSOR_INIT,   // Sensor is initializing, state not yet known
    SENSOR_CLEAR,  // Sensor does not signal anything
    SENSOR_ACTIVE, // Sensor has detected something
    SENSOR_ERROR   // Sensor is gone (hardware issue, e.g. unplugged)
} sensor_state_t;
