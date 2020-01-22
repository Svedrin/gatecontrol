#include <unity.h>
#include "monitoredsensor.h"
#include "test_monitoredsensor.h"

void test_monitoredsensor() {
    TestableMonitoredSensor sensor(1, 2, 0);

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