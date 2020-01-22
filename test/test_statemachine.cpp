#include <unity.h>
#include "statemachine.h"
#include "test_statemachine.h"

/**
 * I'd love to use cucumber for these tests, but since I don't know
 * how to integrate it properly with PlatformIO, I won't.
 * I will however try to make these tests as cucumbery as possible.
 * Let's hope that doesn't end in chaos, as such attempts often do.
 *
 * So first of all, I shall define some steps from which to build
 * the actual tests.
 */

typedef struct {
    StateMachine statemachine;
    esp_state_t  input;
    step_t       result;
} test_context_t;

test_context_t *test_context = NULL;

void setUp() {
    test_context = new test_context_t;
}

/**
 * Step definitions
 */

void given_gate_is_up() {
    test_context->input.sensor_gate_up   = SENSOR_ACTIVE;
    test_context->input.sensor_gate_down = SENSOR_CLEAR;
}

void given_gate_is_moving() {
    test_context->input.sensor_gate_up   = SENSOR_CLEAR;
    test_context->input.sensor_gate_down = SENSOR_CLEAR;
}

void given_gate_is_down() {
    test_context->input.sensor_gate_up   = SENSOR_CLEAR;
    test_context->input.sensor_gate_down = SENSOR_ACTIVE;
}

void when_time_passes(unsigned long millis) {
    test_context->input.millis = millis;
    test_context->result = test_context->statemachine.step(&test_context->input);
}

// This must be a macro so that line numbers are correct
#define then_current_state_is(state) \
    TEST_ASSERT_EQUAL(state, test_context->result.current_state)

/**
 * Da basics
 */

void test_init_open() {
    given_gate_is_up();
    when_time_passes(10);
    then_current_state_is(GATE_OPEN);
}

void test_init_closed() {
    given_gate_is_down();
    when_time_passes(10);
    then_current_state_is(GATE_CLOSED);

    given_gate_is_moving();
    when_time_passes(20);
    then_current_state_is(GATE_UNKNOWN);

    given_gate_is_up();
    when_time_passes(30);
    then_current_state_is(GATE_OPEN);

    given_gate_is_moving();
    when_time_passes(40);
    then_current_state_is(GATE_UNKNOWN);

    given_gate_is_down();
    when_time_passes(50);
    then_current_state_is(GATE_CLOSED);
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
