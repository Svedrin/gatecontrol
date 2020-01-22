#include <unity.h>
#include "test_statemachine.h"
#include "test_monitoredsensor.h"

int main( int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_init_open);
    RUN_TEST(test_init_closed);
    RUN_TEST(test_remote_close_uninterrupted);
    RUN_TEST(test_monitoredsensor);
    UNITY_END();
}
