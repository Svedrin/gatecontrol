#include <unity.h>
#include "statemachine.h"
#include "monitoredsensor.h"

void test_init_open() {
    StateMachine statemachine;

    esp_state_t input1 = {
        .sensor_gate_up    = SENSOR_ACTIVE,
        .sensor_gate_down  = SENSOR_CLEAR,
        .sensor_lb_blocked = SENSOR_CLEAR,
        .sensor_lb_clear   = SENSOR_CLEAR,
        .millis = 0
    };
    step_t step1 = statemachine.step(&input1);
    TEST_ASSERT_EQUAL(GATE_OPEN, step1.current_state);
}

void test_init_closed() {
    StateMachine statemachine;

    esp_state_t input = {
        .sensor_gate_up    = SENSOR_CLEAR,
        .sensor_gate_down  = SENSOR_ACTIVE,
        .sensor_lb_blocked = SENSOR_CLEAR,
        .sensor_lb_clear   = SENSOR_ACTIVE,
        .millis = 0
    };
    step_t step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_CLOSED, step.current_state);

    input.sensor_gate_down = SENSOR_CLEAR;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_UNKNOWN, step.current_state);

    input.sensor_gate_up = SENSOR_ACTIVE;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_OPEN, step.current_state);

    input.sensor_gate_up = SENSOR_CLEAR;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_UNKNOWN, step.current_state);

    input.sensor_gate_down = SENSOR_ACTIVE;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_CLOSED, step.current_state);
}

void test_remote_close_uninterrupted() {
    StateMachine statemachine;

    esp_state_t input = {
        .sensor_gate_up    = SENSOR_ACTIVE,
        .sensor_gate_down  = SENSOR_CLEAR,
        .sensor_lb_blocked = SENSOR_CLEAR,
        .sensor_lb_clear   = SENSOR_ACTIVE,
        .millis = 100
    };
    step_t step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_OPEN, step.current_state);

    cmd_result_t res = statemachine.cmd_close(500);
    TEST_ASSERT_EQUAL(COMMAND_ACCEPTED, res);

    input.millis = 800;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_OPEN, step.current_state);
    TEST_ASSERT_FALSE(step.trigger);

    res = statemachine.cmd_commit(10536);
    TEST_ASSERT_EQUAL(COMMAND_ACCEPTED, res);

    input.millis = 1600;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_OPEN, step.current_state);
    TEST_ASSERT_TRUE(step.trigger);
}

void test_remote_close_light_barrier_blocked() {
    StateMachine statemachine;

    esp_state_t input = {
        .sensor_gate_up    = SENSOR_ACTIVE,
        .sensor_gate_down  = SENSOR_CLEAR,
        .sensor_lb_blocked = SENSOR_ACTIVE,
        .sensor_lb_clear   = SENSOR_CLEAR,
        .millis = 100
    };
    step_t step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_OPEN, step.current_state);

    cmd_result_t res = statemachine.cmd_close(500);
    TEST_ASSERT_EQUAL(COMMAND_ACCEPTED, res);

    input.millis = 800;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_OPEN, step.current_state);
    TEST_ASSERT_FALSE(step.trigger);

    res = statemachine.cmd_commit(10536);
    TEST_ASSERT_EQUAL(COMMAND_ACCEPTED, res);

    input.millis = 1600;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_OPEN, step.current_state);
    TEST_ASSERT_FALSE(step.trigger);
}

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

int main( int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_init_open);
    RUN_TEST(test_init_closed);
    RUN_TEST(test_remote_close_uninterrupted);
    RUN_TEST(test_monitoredsensor);
    UNITY_END();
}

