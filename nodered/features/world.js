const { setWorldConstructor } = require("cucumber");
const { step } = require("cpu");

function Node() {
    return {
        last_status: [],
        last_messages: [],
        status(status) {
            this.last_status.push(status);
        },
        send(msgs) {
            this.last_messages.push(msgs);
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
