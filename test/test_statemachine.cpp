#include <unity.h>
#include "statemachine.h"
#include "test_statemachine.h"

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
