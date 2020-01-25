#pragma GCC diagnostic push  // save compiler settings
#pragma GCC diagnostic error "-Wall"

#include <unity.h>
#include "test_statemachine.h"
#include "test_monitoredsensor.h"

int main( int argc, char **argv) {
    UNITY_BEGIN();

    // MonitoredSensor tests
    RUN_TEST(test_monitoredsensor);
    RUN_TEST(test_both_is_error);
    RUN_TEST(test_error_on_first_reading);

    // StateMachine tests
    RUN_TEST(test_init_open);
    RUN_TEST(test_init_unknown);
    RUN_TEST(test_init_closed);
    RUN_TEST(test_open_blocked_then_up);
    RUN_TEST(test_open_blocked_then_down);
    RUN_TEST(test_remote_close_normal);
    RUN_TEST(test_remote_close_light_barrier_blocked_always);
    RUN_TEST(test_remote_close_when_already_closed);
    RUN_TEST(test_remote_close_when_position_unknown);
    RUN_TEST(test_remote_close_broken_gate);
    RUN_TEST(test_remote_close_user_first);
    RUN_TEST(test_remote_close_blocked_during_commit);
    RUN_TEST(test_remote_close_blocked_before_commit);
    RUN_TEST(test_remote_close_early_commit);
    RUN_TEST(test_remote_close_late_commit);
    RUN_TEST(test_autoclose_from_closed);
    RUN_TEST(test_autoclose_from_unknown);
    RUN_TEST(test_autoclose_from_open);
    RUN_TEST(test_autoclose_cancel);
    RUN_TEST(test_autoclose_fast_gate);
    RUN_TEST(test_autoclose_broken_gate);
    RUN_TEST(test_autoclose_timeout_in_gate_open);
    RUN_TEST(test_autoclose_user_first_then_down);
    RUN_TEST(test_autoclose_user_first_then_up);
    RUN_TEST(test_autoclose_light_barrier_timing);
    RUN_TEST(test_autoclose_ignore_mqtt);
    RUN_TEST(test_blocked_ignore_autoclose);
    RUN_TEST(test_gate_both_up_and_down_error);
    RUN_TEST(test_gate_up_error);
    RUN_TEST(test_gate_down_error);
    RUN_TEST(test_light_barrier_error);
    RUN_TEST(test_autoclose_button_error);


    UNITY_END();
}

#pragma GCC diagnostic pop // Restore compiler settings
