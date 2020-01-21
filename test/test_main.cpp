//#ifdef UNIT_TEST
#include <unity.h>
#include "statemachine.h"

#define LOW 0
#define HIGH 1

void test_init_open() {
    StateMachine statemachine;

    esp_state_t input1 = {
        .pin_up         = LOW,
        .pin_down       = HIGH,
        .pin_lb_blocked = HIGH,
        .pin_lb_clear   = HIGH,
        .millis = 0
    };
    step_t step1 = statemachine.step(&input1);
    TEST_ASSERT_EQUAL(GATE_OPEN, step1.current_state);
}

void test_init_closed() {
    StateMachine statemachine;

    esp_state_t input1 = {
        .pin_up         = HIGH,
        .pin_down       = LOW,
        .pin_lb_blocked = HIGH,
        .pin_lb_clear   = HIGH,
        .millis = 0
    };
    step_t step1 = statemachine.step(&input1);
    TEST_ASSERT_EQUAL(GATE_CLOSED, step1.current_state);

    input1.pin_down = HIGH;
    step_t step2 = statemachine.step(&input1);
    TEST_ASSERT_EQUAL(GATE_UNKNOWN, step2.current_state);
}

int main( int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_init_open);
    RUN_TEST(test_init_closed);
    UNITY_END();
}

//#endif
