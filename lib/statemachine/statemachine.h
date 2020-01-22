typedef enum {
    GATE_INIT,     // State used for the first reading after boot
    GATE_OPEN,     // Gate is full open (top sensor is low)
    GATE_UNKNOWN,  // Gate is somewhere between full open and full closed
    GATE_CLOSED,   // Gate is full closed (bottom sensor is low)
    GATE_ERROR
} state_t;

typedef enum {
    COMMAND_ACCEPTED,
    COMMAND_IGNORED
} cmd_result_t;

typedef enum {
    SENSOR_CLEAR,
    SENSOR_ACTIVE
} sensor_state_t;

typedef struct esp_state_t {
    int sensor_gate_up;
    int sensor_gate_down;
    int sensor_lb_blocked;
    int sensor_lb_clear;
    unsigned long millis;
} esp_state_t;

typedef struct {
    bool trigger;
    state_t previous_state;
    state_t current_state;
} step_t;

class StateMachine {
    protected:
        state_t current_state;
        unsigned long received_close_at;
        unsigned long received_commit_at;

    public:
        StateMachine();
        step_t step(esp_state_t *esp_state);
        cmd_result_t cmd_close(unsigned long received_at);
        cmd_result_t cmd_commit(unsigned long received_at);
};
