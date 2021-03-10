const { setWorldConstructor } = require("@cucumber/cucumber");
const { step } = require("cpu");

function Node() {
    return {
        last_status            : "TEST_FAILED",
        last_lamp_cmd_direct   : "TEST_FAILED",
        last_lamp_cmd_then_off : "TEST_FAILED",
        last_lamp_cmd_blink    : "TEST_FAILED",
        last_gate_cmd          : "TEST_FAILED",
        status(status) {
            this.last_status = status;
        },
        send(msgs) {
            this.last_lamp_cmd_direct   = msgs[0];
            this.last_lamp_cmd_then_off = msgs[1];
            this.last_lamp_cmd_blink    = msgs[2];
            this.last_gate_cmd          = msgs[3];
        }
    };
}

function Context() {
    return {
        vars: {},
        get(key) {
            return this.vars[key];
        },
        set(key, val) {
            this.vars[key] = val;
        }
    };
}

function CustomWorld() {
    this.cmd_light_red    = { on: true, hex: "#DC3545", duration: 100, bri: 60 };
    this.cmd_light_yellow = { on: true, hex: "#efef1a", duration: 100, bri: 60 };
    this.cmd_light_green  = { on: true, hex: "#28A745", duration: 100, bri: 60 };
    this.cmd_light_blue   = { on: true, hex: "#32d2ff", duration: 100, bri: 60 };
    this.node_ctx = Context();
    this.flow_ctx = Context();
    this.node = Node();
    this.step = step;
}

setWorldConstructor(CustomWorld);
