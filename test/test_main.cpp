#pragma GCC diagnostic push  // save compiler settings
#pragma GCC diagnostic error "-Wall"

#include <unity.h>
#include "test_statemachine.h"
#include "test_monitoredsensor.h"

int main( int argc, char **argv) {
    UNITY_BEGIN();

    // MonitoredSensor tests
    RUN_TEST(test_monitoredsensor);

    // StateMachine tests
    RUN_TEST(test_init_open);
    RUN_TEST(test_init_unknown);
    RUN_TEST(test_init_closed);
    RUN_TEST(test_open_blocked_then_up);
    RUN_TEST(test_open_blocked_then_down);
    RUN_TEST(test_remote_close_normal);
    RUN_TEST(test_remote_close_light_barrier_blocked);
    RUN_TEST(test_remote_close_when_already_closed);
    RUN_TEST(test_remote_close_when_position_unknown);
    RUN_TEST(test_remote_close_broken_gate);
    RUN_TEST(test_remote_close_user_first);
    RUN_TEST(test_autoclose_normal);
    RUN_TEST(test_autoclose_cancel);
    RUN_TEST(test_autoclose_fast_gate);
    RUN_TEST(test_autoclose_broken_gate);
    RUN_TEST(test_autoclose_from_open);
    RUN_TEST(test_autoclose_from_unknown);
    RUN_TEST(test_autoclose_timeout);

    UNITY_END();
}

#pragma GCC diagnostic pop // Restore compiler settings
