#pragma GCC diagnostic push  // save compiler settings
#pragma GCC diagnostic error "-Wall"

#include <unity.h>
#include "monitoredsensor.h"
#include "test_monitoredsensor.h"

#define ACTIVE 0
#define DOWN 1

void test_monitoredsensor() {
    TestableMonitoredSensor sensor(1, 2, 0, false);

    // Test valid states
    sensor.set_pin_states(0, 0);
    TEST_ASSERT_EQUAL(SENSOR_ACTIVE, sensor.read(40));

    sensor.set_pin_states(1, 0);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, sensor.read(60));

    // Create an error -- should be debounced until millis=130
    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, sensor.read(80));

    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, sensor.read(100));

    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, sensor.read(120));

    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_ERROR, sensor.read(140));

    // Go back to active state, check the same thing with CLEAR
    sensor.set_pin_states(0, 0);
    TEST_ASSERT_EQUAL(SENSOR_ACTIVE, sensor.read(160));

    // Error, should be debounced until millis=230
    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_ACTIVE, sensor.read(180));

    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_ACTIVE, sensor.read(200));

    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_ACTIVE, sensor.read(220));

    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_ERROR, sensor.read(240));

    // Check the relay case: Active=LOW, Monitor=HIGH because
    // monitor is connected to the idle-closed relay port
    // Go back to active state, check that short errors are debounced correctly
    sensor.set_pin_states(0, 1);
    TEST_ASSERT_EQUAL(SENSOR_ACTIVE, sensor.read(260));

    // Error, should be debounced until millis=330
    // The relay is in the process of switching (and for some reason, that takes 40ms)
    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_ACTIVE, sensor.read(280));

    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_ACTIVE, sensor.read(300));

    // Resolve error: The relay has now reached its target position
    sensor.set_pin_states(1, 0);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, sensor.read(320));

    sensor.set_pin_states(1, 0);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, sensor.read(340));

    // The relay is in the process of switching back
    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, sensor.read(360));

    sensor.set_pin_states(1, 1);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, sensor.read(380));

    // Resolve error: The relay has now reached its target position
    sensor.set_pin_states(0, 1);
    TEST_ASSERT_EQUAL(SENSOR_ACTIVE, sensor.read(400));
}

void test_both_is_error() {
    // Test both_is_error feature
    TestableMonitoredSensor relay(1, 2, ACTIVE, true);

    // Valid: Active pin signalling, monitor pin down
    relay.set_pin_states(ACTIVE, DOWN);
    TEST_ASSERT_EQUAL(SENSOR_ACTIVE, relay.read(100));

    // Valid: Active pin down, monitor pin signalling
    relay.set_pin_states(DOWN, ACTIVE);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, relay.read(200));

    // Error: Both pins are signalling
    relay.set_pin_states(ACTIVE, ACTIVE);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, relay.read(300));
    TEST_ASSERT_EQUAL(SENSOR_ERROR, relay.read(400));

    // Put in a valid reading so that we can observe a state change
    relay.set_pin_states(DOWN, ACTIVE);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, relay.read(500));

    // Error: Both pins are down
    relay.set_pin_states(DOWN, DOWN);
    TEST_ASSERT_EQUAL(SENSOR_CLEAR, relay.read(600));
    TEST_ASSERT_EQUAL(SENSOR_ERROR, relay.read(700));
}

void test_error_on_first_reading() {
    // Test both_is_error feature
    TestableMonitoredSensor relay(1, 2, ACTIVE, true);

    // Valid: Active pin signalling, monitor pin down
    relay.set_pin_states(DOWN, DOWN);
    TEST_ASSERT_EQUAL(SENSOR_ERROR, relay.read(100));
}

#pragma GCC diagnostic pop // Restore compiler settings
