const { setWorldConstructor } = require("cucumber");
const { step } = require("cpu");

const Node = {
    vars: {
        status: null,
        last_messages: null
    },
    status(status) {
        this.status = status;
    },
    send(msgs) {
        this.last_messages = msgs;
    }
};

const Context = {
    vars: {},
    get(key) {
        return this.vars[key];
    },
    set(key, val) {
        this.vars[key] = val;
    }
};

function CustomWorld() {
    this.cmd_light_red    = { on: true, hex: "#DC3545", duration: 100, bri: 60 };
    this.cmd_light_yellow = { on: true, hex: "#efef1a", duration: 100, bri: 60 };
    this.cmd_light_green  = { on: true, hex: "#28A745", duration: 100, bri: 60 };
    this.cmd_light_blue   = { on: true, hex: "#32d2ff", duration: 100, bri: 60 };
    this.node_ctx = Object.create(Context);
    this.flow_ctx = Object.create(Context);
    this.node = Object.create(Node);
    this.step = step;
}

setWorldConstructor(CustomWorld);
