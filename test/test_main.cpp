//#ifdef UNIT_TEST
#include <unity.h>
#include "statemachine.h"

#define LOW 0
#define HIGH 1

void test_init_open() {
    StateMachine statemachine;

    esp_state_t input1 = {
        .sensor_gate_up    = LOW,
        .sensor_gate_down  = HIGH,
        .sensor_lb_blocked = HIGH,
        .sensor_lb_clear   = HIGH,
        .millis = 0
    };
    step_t step1 = statemachine.step(&input1);
    TEST_ASSERT_EQUAL(GATE_OPEN, step1.current_state);
}

void test_init_closed() {
    StateMachine statemachine;

    esp_state_t input = {
        .sensor_gate_up    = HIGH,
        .sensor_gate_down  = LOW,
        .sensor_lb_blocked = HIGH,
        .sensor_lb_clear   = LOW,
        .millis = 0
    };
    step_t step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_CLOSED, step.current_state);

    input.sensor_gate_down = HIGH;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_UNKNOWN, step.current_state);

    input.sensor_gate_up = LOW;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_OPEN, step.current_state);

    input.sensor_gate_up = HIGH;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_UNKNOWN, step.current_state);

    input.sensor_gate_down = LOW;
    step = statemachine.step(&input);
    TEST_ASSERT_EQUAL(GATE_CLOSED, step.current_state);
}

void test_remote_close_uninterrupted() {
    StateMachine statemachine;

    esp_state_t input = {
        .sensor_gate_up    = LOW,
        .sensor_gate_down  = HIGH,
        .sensor_lb_blocked = HIGH,
        .sensor_lb_clear   = LOW,
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

int main( int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_init_open);
    RUN_TEST(test_init_closed);
    RUN_TEST(test_remote_close_uninterrupted);
    UNITY_END();
}

//#endif
