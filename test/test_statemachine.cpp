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

void given_autoclose_button_is_pressed() {
    test_context->input.button_autoclose = SENSOR_ACTIVE;
}

void given_autoclose_button_is_released() {
    test_context->input.button_autoclose = SENSOR_CLEAR;
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

#define then_autoclose_is(pending_or_triggered) \
    TEST_ASSERT_EQUAL(pending_or_triggered, test_context->result.autoclose_state);

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

void test_init_unknown() {
    given_gate_is_moving();
    then_current_state_is(GATE_INIT);

    when_time_passes(10);
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

void test_open_blocked_then_up() {
    given_gate_is_up();
    when_time_passes(10);
    then_current_state_is(GATE_OPEN);

    given_light_barrier_is_blocked();
    when_time_passes(20);
    then_current_state_is(GATE_BLOCKED);

    given_light_barrier_is_clear();
    when_time_passes(30);
    then_current_state_is(GATE_OPEN);

    given_light_barrier_is_blocked();
    when_time_passes(40);
    then_current_state_is(GATE_BLOCKED);

    given_gate_is_moving();
    when_time_passes(50);
    then_current_state_is(GATE_UNKNOWN);

    given_light_barrier_is_clear();
    when_time_passes(60);
    then_current_state_is(GATE_UNKNOWN);

    given_gate_is_up();
    when_time_passes(70);
    then_current_state_is(GATE_OPEN);

    when_time_passes(80);
    then_current_state_is(GATE_OPEN);
}

void test_open_blocked_then_down() {
    given_gate_is_up();
    when_time_passes(10);
    then_current_state_is(GATE_OPEN);

    given_light_barrier_is_blocked();
    when_time_passes(20);
    then_current_state_is(GATE_BLOCKED);

    given_light_barrier_is_clear();
    when_time_passes(30);
    then_current_state_is(GATE_OPEN);

    given_light_barrier_is_blocked();
    when_time_passes(40);
    then_current_state_is(GATE_BLOCKED);

    given_gate_is_moving();
    when_time_passes(50);
    then_current_state_is(GATE_UNKNOWN);

    given_light_barrier_is_clear();
    when_time_passes(60);
    then_current_state_is(GATE_UNKNOWN);

    given_gate_is_down();
    when_time_passes(70);
    then_current_state_is(GATE_CLOSED);

    when_time_passes(80);
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
void test_remote_close_light_barrier_blocked_always() {
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

/**
 * Chapter three: Remote close, edge cases.
 *
 * Let's make sure that when weird stuff happens, we do NOT react
 * in a dangerous manner.
 */

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

void test_remote_close_broken_gate() {
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    when_mqtt_close_command_arrives_at(500);
    when_time_passes(800);
    when_mqtt_commit_command_arrives_at(10536);
    when_time_passes(1600);
    then_current_state_is(GATE_CLOSE_TRIGGERED);
    then_we_trigger();

    when_time_passes(1700);
    then_current_state_is(GATE_CLOSE_TRIGGERED);

    when_time_passes(1800 + GATE_ERROR_TIMEOUT);
    then_current_state_is(GATE_ERROR);
}

// Remote close is ongoing, but still someone just closes the gate manually.
void test_remote_close_user_first() {
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    when_mqtt_close_command_arrives_at(500);
    when_time_passes(800);
    then_current_state_is(GATE_CLOSE_PREPARE);
    then_we_do_not_trigger();

    // User pushes da button
    given_gate_is_moving();
    when_time_passes(900);
    then_current_state_is(GATE_UNKNOWN);

    // Scotty doesn't know, so he sends his COMMIT
    when_mqtt_commit_command_arrives_at(10536);
    then_the_command_is(COMMAND_IGNORED);

    when_time_passes(1600);
    then_current_state_is(GATE_UNKNOWN);
    then_we_do_not_trigger();

    given_gate_is_down();
    when_time_passes(1700);
    then_current_state_is(GATE_CLOSED);
}

void test_remote_close_blocked_during_commit() {
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    when_mqtt_close_command_arrives_at(500);
    when_time_passes(800);
    then_current_state_is(GATE_CLOSE_PREPARE);
    then_we_do_not_trigger();

    // User walks into the light barrier
    given_light_barrier_is_blocked();
    when_time_passes(900);
    then_current_state_is(GATE_BLOCKED);

    // Scotty doesn't know, so he sends his COMMIT
    when_mqtt_commit_command_arrives_at(10536);
    then_the_command_is(COMMAND_IGNORED);

    when_time_passes(1600);
    then_current_state_is(GATE_BLOCKED);
    then_we_do_not_trigger();

    given_light_barrier_is_clear();
    when_time_passes(1700);
    then_current_state_is(GATE_OPEN);
}

void test_remote_close_blocked_before_commit() {
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    when_mqtt_close_command_arrives_at(500);
    when_time_passes(800);
    then_current_state_is(GATE_CLOSE_PREPARE);
    then_we_do_not_trigger();

    // User walks into the light barrier
    given_light_barrier_is_blocked();
    when_time_passes(900);
    then_current_state_is(GATE_BLOCKED);

    given_light_barrier_is_clear();
    when_time_passes(1000);
    then_current_state_is(GATE_OPEN);

    // Scotty doesn't know, so he sends his COMMIT
    when_mqtt_commit_command_arrives_at(10536);
    then_the_command_is(COMMAND_IGNORED);

    when_time_passes(1600);
    then_current_state_is(GATE_OPEN);
    then_we_do_not_trigger();

    when_time_passes(1700);
    then_current_state_is(GATE_OPEN);
}

// COMMIT arrives too early.
void test_remote_close_early_commit() {
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    when_mqtt_close_command_arrives_at(500);
    when_time_passes(800);
    then_current_state_is(GATE_CLOSE_PREPARE);
    then_we_do_not_trigger();

    // Test a broken flow first: Commit arrives 5s too early
    when_mqtt_commit_command_arrives_at(5536);
    // TODO then_the_command_is(COMMAND_IGNORED);

    when_time_passes(5600);
    then_current_state_is(GATE_OPEN);
    then_we_do_not_trigger();

    when_time_passes(10536);
    then_current_state_is(GATE_OPEN);
    then_we_do_not_trigger();

    when_time_passes(15536);
    then_current_state_is(GATE_OPEN);
    then_we_do_not_trigger();

    // Now test a normal flow again to see that it still works
    when_mqtt_close_command_arrives_at(20500);
    when_time_passes(20800);
    then_current_state_is(GATE_CLOSE_PREPARE);
    then_we_do_not_trigger();

    when_mqtt_commit_command_arrives_at(30536);
    then_the_command_is(COMMAND_ACCEPTED);

    when_time_passes(30600);
    then_current_state_is(GATE_CLOSE_TRIGGERED);
    then_we_trigger();

    given_gate_is_moving();
    when_time_passes(30700);
    then_current_state_is(GATE_UNKNOWN);

    given_gate_is_down();
    when_time_passes(30800);
    then_current_state_is(GATE_CLOSED);
}

// COMMIT arrives too late.
void test_remote_close_late_commit() {
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    when_mqtt_close_command_arrives_at(500);
    when_time_passes(800);
    then_current_state_is(GATE_CLOSE_PREPARE);
    then_we_do_not_trigger();

    // Test a broken flow first: Commit arrives 5s too late
    when_mqtt_commit_command_arrives_at(15536);
    // TODO then_the_command_is(COMMAND_IGNORED);

    when_time_passes(15600);
    then_current_state_is(GATE_OPEN);
    then_we_do_not_trigger();

    when_time_passes(20536);
    then_current_state_is(GATE_OPEN);
    then_we_do_not_trigger();

    when_time_passes(20536);
    then_current_state_is(GATE_OPEN);
    then_we_do_not_trigger();

    // Now test a normal flow again to see that it still works
    when_mqtt_close_command_arrives_at(25500);
    when_time_passes(25800);
    then_current_state_is(GATE_CLOSE_PREPARE);
    then_we_do_not_trigger();

    when_mqtt_commit_command_arrives_at(35536);
    then_the_command_is(COMMAND_ACCEPTED);

    when_time_passes(35600);
    then_current_state_is(GATE_CLOSE_TRIGGERED);
    then_we_trigger();

    given_gate_is_moving();
    when_time_passes(35700);
    then_current_state_is(GATE_UNKNOWN);

    given_gate_is_down();
    when_time_passes(35800);
    then_current_state_is(GATE_CLOSED);
}



/**
 * Chapter four: Autoclose, non-edge cases.
 *
 * This operation is basically an extension of chapter two.
 */

void test_autoclose_from_closed() {
    given_gate_is_down();
    when_time_passes(10);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_OFF);

    given_autoclose_button_is_pressed();
    when_time_passes(20);
    then_current_state_is(GATE_OPEN_TRIGGERED);
    then_we_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    when_time_passes(25);
    then_current_state_is(GATE_OPEN_TRIGGERED);
    then_we_do_not_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    given_autoclose_button_is_released();
    when_time_passes(30);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_ON);

    given_gate_is_moving();
    when_time_passes(40);
    then_current_state_is(GATE_UNKNOWN);
    then_autoclose_is(AUTOCLOSE_ON);

    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(50);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_ON);

    given_light_barrier_is_blocked();
    when_time_passes(60);
    then_current_state_is(GATE_BLOCKED);
    then_autoclose_is(AUTOCLOSE_ON);

    given_light_barrier_is_clear();
    when_time_passes(70);
    then_current_state_is(GATE_CLOSE_AUTO);
    then_autoclose_is(AUTOCLOSE_PENDING);

    when_time_passes(15080);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_TRIGGERED);

    when_mqtt_close_command_arrives_at(15210);
    then_the_command_is(COMMAND_ACCEPTED);

    when_time_passes(15300);
    then_current_state_is(GATE_CLOSE_PREPARE);
    then_we_do_not_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    when_mqtt_commit_command_arrives_at(25236);
    then_the_command_is(COMMAND_ACCEPTED);

    when_time_passes(25300);
    then_current_state_is(GATE_CLOSE_TRIGGERED);
    then_we_trigger();
    then_autoclose_is(AUTOCLOSE_OFF);

    given_gate_is_moving();
    when_time_passes(25400);
    then_current_state_is(GATE_UNKNOWN);
    then_autoclose_is(AUTOCLOSE_OFF);

    given_gate_is_down();
    when_time_passes(30000);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_OFF);
}

void test_autoclose_from_unknown() {
    given_gate_is_down();
    when_time_passes(10);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_OFF);

    given_gate_is_moving();
    when_time_passes(20);
    then_current_state_is(GATE_UNKNOWN);
    then_autoclose_is(AUTOCLOSE_OFF);

    given_autoclose_button_is_pressed();
    when_time_passes(30);
    then_current_state_is(GATE_UNKNOWN);
    then_autoclose_is(AUTOCLOSE_ON);
    then_we_do_not_trigger();

    given_autoclose_button_is_released();
    when_time_passes(35);
    then_current_state_is(GATE_UNKNOWN);
    then_autoclose_is(AUTOCLOSE_ON);
    then_we_do_not_trigger();

    given_gate_is_moving();
    when_time_passes(40);
    then_current_state_is(GATE_UNKNOWN);
    then_autoclose_is(AUTOCLOSE_ON);

    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(50);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_ON);
}

void test_autoclose_from_open() {
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_OFF);

    given_autoclose_button_is_pressed();
    when_time_passes(20);
    then_current_state_is(GATE_OPEN);
    then_we_do_not_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    when_time_passes(25);
    then_current_state_is(GATE_OPEN);
    then_we_do_not_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    given_autoclose_button_is_released();
    when_time_passes(30);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_ON);

    // From here on out, everything works just as in _from_closed()

    given_light_barrier_is_blocked();
    when_time_passes(60);
    then_current_state_is(GATE_BLOCKED);
    then_autoclose_is(AUTOCLOSE_ON);

    given_light_barrier_is_clear();
    when_time_passes(70);
    then_current_state_is(GATE_CLOSE_AUTO);
    then_autoclose_is(AUTOCLOSE_PENDING);

    when_time_passes(15080);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_TRIGGERED);

    when_mqtt_close_command_arrives_at(15210);
    then_the_command_is(COMMAND_ACCEPTED);

    when_time_passes(15300);
    then_current_state_is(GATE_CLOSE_PREPARE);
    then_we_do_not_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    when_mqtt_commit_command_arrives_at(25236);
    then_the_command_is(COMMAND_ACCEPTED);

    when_time_passes(25300);
    then_current_state_is(GATE_CLOSE_TRIGGERED);
    then_we_trigger();
    then_autoclose_is(AUTOCLOSE_OFF);

    given_gate_is_moving();
    when_time_passes(25400);
    then_current_state_is(GATE_UNKNOWN);
    then_autoclose_is(AUTOCLOSE_OFF);

    given_gate_is_down();
    when_time_passes(30000);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_OFF);
}


/**
 * Chapter five: Autoclose, edge cases.
 *
 * Same intention as chapter three: Weird stuff going on, make sure
 * we don't make it worse.
 */

void test_autoclose_cancel() {
    given_gate_is_down();
    when_time_passes(10);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_OFF);

    given_autoclose_button_is_pressed();
    when_time_passes(20);
    then_current_state_is(GATE_OPEN_TRIGGERED);
    then_we_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    given_autoclose_button_is_released();
    when_time_passes(30);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_ON);

    given_gate_is_moving();
    when_time_passes(40);
    then_current_state_is(GATE_UNKNOWN);
    then_autoclose_is(AUTOCLOSE_ON);

    // The user now presses the gate engine's switch
    // to stop and close the gate

    given_gate_is_down();
    when_time_passes(50);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_OFF);

    // The state above got reported by the code that ran
    // the state change; check that it also persisted it
    when_time_passes(60);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_OFF);
}

void test_autoclose_fast_gate() {
    // Test for when gate starts moving before the button is released.
    given_gate_is_down();
    when_time_passes(10);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_OFF);

    given_autoclose_button_is_pressed();
    when_time_passes(20);
    then_current_state_is(GATE_OPEN_TRIGGERED);
    then_we_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    when_time_passes(25);
    then_current_state_is(GATE_OPEN_TRIGGERED);
    then_we_do_not_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    given_gate_is_moving();
    when_time_passes(30);
    then_current_state_is(GATE_UNKNOWN);
    then_autoclose_is(AUTOCLOSE_ON);

    given_autoclose_button_is_released();
    when_time_passes(40);
    then_current_state_is(GATE_UNKNOWN);
    then_autoclose_is(AUTOCLOSE_ON);

    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(50);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_ON);

    // From here on out, everything works just as in _normal,
    // so not testing again
}

void test_autoclose_broken_gate() {
    // Test for when gate won't start moving
    given_gate_is_down();
    when_time_passes(10);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_OFF);

    given_autoclose_button_is_pressed();
    when_time_passes(20);
    then_current_state_is(GATE_OPEN_TRIGGERED);
    then_we_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    when_time_passes(25);
    then_current_state_is(GATE_OPEN_TRIGGERED);
    then_we_do_not_trigger();
    then_autoclose_is(AUTOCLOSE_ON);

    when_time_passes(30);
    then_current_state_is(GATE_OPEN_TRIGGERED);
    then_autoclose_is(AUTOCLOSE_ON);

    when_time_passes(40 + GATE_ERROR_TIMEOUT);
    then_current_state_is(GATE_ERROR);
}

void test_autoclose_timeout_in_gate_open() {
    // Autoclose starts from GATE_BLOCKED, check it times out
    // correctly when that state is never reached.
    // Enable autoclose while the gate's open (shorter)
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    given_autoclose_button_is_pressed();
    when_time_passes(20);
    given_autoclose_button_is_released();
    when_time_passes(30);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_ON);

    // Now we just sit idle for a while

    when_time_passes(50 + AUTOCLOSE_TIMEOUT);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_OFF);

    // The state above got reported by the code that ran
    // the state change; check that it also persisted it
    when_time_passes(60 + AUTOCLOSE_TIMEOUT);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_OFF);
}

void test_autoclose_user_first_then_down() {
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    given_autoclose_button_is_pressed();
    when_time_passes(20);
    given_autoclose_button_is_released();
    when_time_passes(30);
    then_autoclose_is(AUTOCLOSE_ON);
    given_light_barrier_is_blocked();
    when_time_passes(40);
    given_light_barrier_is_clear();
    when_time_passes(50);
    then_current_state_is(GATE_CLOSE_AUTO);

    // Now the user presses the gate engine remote to
    // close the gate
    given_gate_is_moving();
    when_time_passes(60);
    then_current_state_is(GATE_UNKNOWN);
    then_we_do_not_trigger();
    then_autoclose_is(AUTOCLOSE_OFF);

    given_gate_is_down();
    when_time_passes(70);
    then_current_state_is(GATE_CLOSED);
    then_autoclose_is(AUTOCLOSE_OFF);
}

void test_autoclose_user_first_then_up() {
    given_gate_is_up();
    given_light_barrier_is_clear();
    when_time_passes(10);
    given_autoclose_button_is_pressed();
    when_time_passes(20);
    given_autoclose_button_is_released();
    when_time_passes(30);
    then_autoclose_is(AUTOCLOSE_ON);
    given_light_barrier_is_blocked();
    when_time_passes(40);
    given_light_barrier_is_clear();
    when_time_passes(50);
    then_current_state_is(GATE_CLOSE_AUTO);

    // Now the user presses the gate engine remote to
    // close the gate
    given_gate_is_moving();
    when_time_passes(60);
    then_current_state_is(GATE_UNKNOWN);
    then_we_do_not_trigger();
    then_autoclose_is(AUTOCLOSE_OFF);

    // Now they change their mind and move the gate back up
    given_gate_is_up();
    when_time_passes(70);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_OFF);

    when_time_passes(80);
    then_current_state_is(GATE_OPEN);
    then_autoclose_is(AUTOCLOSE_OFF);
}

#pragma GCC diagnostic pop // Restore compiler settings
