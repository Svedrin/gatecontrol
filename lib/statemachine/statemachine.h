#include <sensorstate.h>

// After autoclose is triggered, users need to block the light barrier once
// for the whole process to start. How long do we wait for that to happen
// before we cancel the whole process?
#define AUTOCLOSE_TIMEOUT 60000

// How long do we wait after the light barrier becomes clear, before we
// initiate the close procedure?
#define AUTOCLOSE_WAIT_PERIOD 15000

// When we trigger the gate, after how much time do we expect it to get moving?
#define GATE_ERROR_TIMEOUT 5000

// When we receive a CLOSE command, after which time period do we expect a COMMIT?
#define CLOSE_TILL_COMMIT 10000

typedef enum {
    GATE_INIT,            // State used for the first reading after boot
    GATE_OWAIT,           // Gate is full open (top sensor is low), but we don't trust the reading just yet
    GATE_OPEN,            // Gate is full open (top sensor is low)
    GATE_UNKNOWN,         // Gate is somewhere between full open and full closed
    GATE_CLOSED,          // Gate is full closed (bottom sensor is low)
    GATE_BLOCKED,         // Light barrier indicates there's something in the way
    GATE_CLOSE_AUTO,      // Autoclose is enabled, timeout not yet expired
    GATE_CLOSE_PREPARE,   // We received a CLOSE command, waiting for COMMIT
    GATE_CLOSE_TRIGGERED, // COMMIT happened, gate triggered, waiting for it to move
    GATE_OPEN_TRIGGERED,  // AutoClose button pushed when gate closed -> open
    GATE_ERROR
} state_t;

typedef enum {
    COMMAND_IGNORED,
    COMMAND_ACCEPTED
} cmd_result_t;

typedef struct esp_state_t {
    sensor_state_t sensor_gate_up;
    sensor_state_t sensor_gate_down;
    sensor_state_t sensor_light_barrier;
    sensor_state_t button_autoclose;
    unsigned long millis;
} esp_state_t;

typedef enum {
    AUTOCLOSE_OFF,        // Autoclose is not enabled, nothing will happen.
    AUTOCLOSE_ENABLED,    // Autoclose has just been enabled.
    AUTOCLOSE_ON,         // Autoclose is enabled, but not currently active.
    AUTOCLOSE_PENDING,    // Autoclose is enabled and the clock has started ticking.
    AUTOCLOSE_RESET,      // Autoclose is enabled but the clock has been reset because the Light Barrier is blocked.
    AUTOCLOSE_TRIGGERED   // Autoclose is enabled and the clock has expired, thus the closing procedure shall be initiated.
} autoclose_state_t;

typedef struct {
    bool trigger;
    autoclose_state_t autoclose_state;
    state_t previous_state;
    state_t current_state;
} step_t;

class StateMachine {
    protected:
        state_t current_state;
        unsigned long entered_owait_at;
        unsigned long received_close_at;
        unsigned long received_commit_at;
        unsigned long autoclose_timer_started_at;
        unsigned long triggered_at;
        unsigned long autoclose_enabled_at;

    public:
        StateMachine();
        step_t step(esp_state_t *esp_state);
        cmd_result_t cmd_close(unsigned long received_at);
        cmd_result_t cmd_commit(unsigned long received_at);
};
