void test_init_open();
void test_init_unknown();
void test_init_closed();
void test_open_blocked_then_up();
void test_open_blocked_then_down();
void test_remote_close_normal();
void test_remote_close_light_barrier_blocked_always();
void test_remote_close_when_already_closed();
void test_remote_close_when_position_unknown();
void test_remote_close_broken_gate();
void test_remote_close_user_first();
void test_remote_close_blocked_during_commit();
void test_remote_close_blocked_before_commit();
void test_remote_close_early_commit();
void test_remote_close_late_commit();
void test_autoclose_from_closed();
void test_autoclose_from_unknown();
void test_autoclose_from_open();
void test_autoclose_cancel();
void test_autoclose_fast_gate();
void test_autoclose_broken_gate();
void test_autoclose_timeout_in_gate_open();
void test_autoclose_user_first_then_down();
void test_autoclose_user_first_then_up();
void test_autoclose_light_barrier_timing();
void test_autoclose_ignore_mqtt();
void test_blocked_ignore_autoclose();
