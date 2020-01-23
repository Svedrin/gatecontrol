#pragma GCC diagnostic push  // save compiler settings
#pragma GCC diagnostic error "-Wall"

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
    cmd_result_t cmd_result;
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

void given_light_barrier_is_clear() {
    test_context->input.sensor_light_barrier = SENSOR_CLEAR;
}

void given_light_barrier_is_blocked() {
    test_context->input.sensor_light_barrier = SENSOR_ACTIVE;
}

void given_light_barrier_is_broken() {
    test_context->input.sensor_light_barrier = SENSOR_ERROR;
}

void when_time_passes(unsigned long millis) {
    test_context->input.millis = millis;
    test_context->result = test_context->statemachine.step(&test_context->input);
}

void when_mqtt_close_command_arrives_at(unsigned long millis) {
    test_context->cmd_result = test_context->statemachine.cmd_close(millis);
}

void when_mqtt_commit_command_arrives_at(unsigned long millis) {
    test_context->cmd_result = test_context->statemachine.cmd_commit(millis);
}

// This must be a macro so that line numbers are correct
#define then_current_state_is(state) \
    TEST_ASSERT_EQUAL(state, test_context->result.current_state)

#define then_the_command_is(accepted_or_rejected) \
    TEST_ASSERT_EQUAL(accepted_or_rejected, test_context->cmd_result);

#define then_we_do_not_trigger() \
    TEST_ASSERT_FALSE(test_context->result.trigger)

#define then_we_trigger() \
    TEST_ASSERT_TRUE(test_context->result.trigger)

/**
 * Chapter one: Da basics.
 *
 * Let's check that simple state tracking works.
 */

void test_init_open() {
    given_gate_is_up();
    then_current_state_is(GATE_INIT);

    when_time_passes(10);
    then_current_state_is(GATE_OPEN);

    given_gate_is_moving();
    when_time_passes(20);
    then_current_state_is(GATE_UNKNOWN);

    given_gate_is_down();
    when_time_passes(30);
    then_current_state_is(GATE_CLOSED);

    given_gate_is_moving();
    when_time_passes(40);
    then_current_state_is(GATE_UNKNOWN);

    given_gate_is_up();
    when_time_passes(50);
    then_current_state_is(GATE_OPEN);
}

void test_init_closed() {
    given_gate_is_down();
    then_current_state_is(GATE_INIT);

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

/**
 * Chapter two: Remote close, non-edge cases.
 *
 * This operation requires a two-phase commit, where a central orchestrator
 * first sends a "CLOSE" command, followed by a "COMMIT" ~10 seconds later.
 * We rely on them to issue appropriate warnings in the meantime.
 * Let's make sure this works correctly, but let's not be mean just yet.
 */

void test_remote_close_normal() {
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    then_current_state_is(GATE_OPEN);

    when_mqtt_close_command_arrives_at(500);
    then_the_command_is(COMMAND_ACCEPTED);

    when_time_passes(800);
    then_current_state_is(GATE_CLOSE_PREPARE);
    then_we_do_not_trigger();

    when_mqtt_commit_command_arrives_at(10536);
    then_the_command_is(COMMAND_ACCEPTED);

    when_time_passes(1600);
    then_current_state_is(GATE_CLOSE_TRIGGERED);
    then_we_trigger();

    given_gate_is_moving();
    when_time_passes(1700);
    then_current_state_is(GATE_UNKNOWN);

    given_gate_is_down();
    when_time_passes(1800);
    then_current_state_is(GATE_CLOSED);
}

// When the light barrier is blocked, we must not trigger.
void test_remote_close_light_barrier_blocked() {
    given_gate_is_up();
    given_light_barrier_is_blocked();
    when_time_passes(100);
    then_current_state_is(GATE_OPEN);
    when_time_passes(200);
    then_current_state_is(GATE_BLOCKED);

    when_mqtt_close_command_arrives_at(500);
    then_the_command_is(COMMAND_IGNORED);

    when_time_passes(800);
    then_current_state_is(GATE_BLOCKED);
    then_we_do_not_trigger();

    when_mqtt_commit_command_arrives_at(10536);
    then_the_command_is(COMMAND_IGNORED);

    when_time_passes(1600);
    then_current_state_is(GATE_BLOCKED);
    then_we_do_not_trigger();
}

// When the gate is closed, we must not trigger (and thereby open it)
void test_remote_close_when_already_closed() {
    given_gate_is_down();
    given_light_barrier_is_clear();
    when_time_passes(10);
    then_current_state_is(GATE_CLOSED);

    when_mqtt_close_command_arrives_at(500);
    then_the_command_is(COMMAND_IGNORED);

    when_time_passes(800);
    then_current_state_is(GATE_CLOSED);
    then_we_do_not_trigger();

    when_mqtt_commit_command_arrives_at(10536);
    then_the_command_is(COMMAND_IGNORED);

    when_time_passes(1600);
    then_current_state_is(GATE_CLOSED);
    then_we_do_not_trigger();
}

// When the gate position is unknown, we must not trigger
void test_remote_close_when_position_unknown() {
    given_gate_is_moving();
    given_light_barrier_is_clear();
    when_time_passes(10);
    then_current_state_is(GATE_UNKNOWN);

    when_mqtt_close_command_arrives_at(500);
    then_the_command_is(COMMAND_IGNORED);

    when_time_passes(800);
    then_current_state_is(GATE_UNKNOWN);
    then_we_do_not_trigger();

    when_mqtt_commit_command_arrives_at(10536);
    then_the_command_is(COMMAND_IGNORED);

    when_time_passes(1600);
    then_current_state_is(GATE_UNKNOWN);
    then_we_do_not_trigger();
}

#pragma GCC diagnostic pop // Restore compiler settings
