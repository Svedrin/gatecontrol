function step(context, node, msg) {
//   ----------------->8---------- CUT HERE ----------------------------------
/**
 * Node outputs:
 *  1. LIFX Light (directly)
 *  2. LIFX Light via a trigger node that sends "off" 120 seconds after our command
 *  3. LIFX Light via a function node that lets it blink 5 times, then static on
 *     see blink-then-on.js
 *  4. Gate controller set_hard_position via
 *     * a trigger node that sends "COMMIT" 10 seconds after our command,
 *     * a function node that only allows the COMMIT to go through if it
 *       received messages on the lifx/ampel topic in the last 10 seconds
 *       see commit-guard.js
 */

var signal_light_topic = "lifx/ampel";

var cmd_light_red    = { on: true, hex: "#DC3545", duration: 100, bri: 60 };
var cmd_light_yellow = { on: true, hex: "#efef1a", duration: 100, bri: 60 };
var cmd_light_green  = { on: true, hex: "#28A745", duration: 100, bri: 60 };
var cmd_light_blue   = { on: true, hex: "#32d2ff", duration: 100, bri: 60 };
var status_open      = { fill: "red",    shape: "dot", text: "open"    };
var status_unknown   = { fill: "yellow", shape: "dot", text: "unknown" };
var status_closing   = { fill: "red",    shape: "dot", text: "closing" };
var status_closed    = { fill: "green",  shape: "dot", text: "closed"  };
var status_blocked   = { fill: "yellow", shape: "dot", text: "blocked" };
var status_autoclose = { fill: "blue",   shape: "dot", text: "open+autoclose" };

// Input: Let's see what kinda message we've got

var ctrl_topic   = "<none>";
var node_status  = null;
var lamp_command = null;
var lamp_mode    = "direct"; // others: "cmd-then-off", "blink"
var gate_command = null;

if (msg.topic.startsWith("ctrl/")) {
    // "ctrl/a1234b/something" -> "something"
    ctrl_topic = msg.topic.split("/")[2];
}

var current_state = context.get("state") || "UNKNOWN";
var autoclose_on  = context.get("autoclose_on") || false;

switch (current_state) {
    case "UNKNOWN":
        node_status = status_unknown;
        if (ctrl_topic == "current_hard_position") {
            switch (msg.payload) {
                case "CLOSED":
                    node_status   = status_closed;
                    lamp_command  = cmd_light_red;
                    lamp_mode     = "cmd-then-off";
                    current_state = "CLOSED";
                    context.set("autoclose_on", false);
                    break;
                case "BLOCKED":
                    node_status   = status_blocked;
                    lamp_command  = cmd_light_yellow;
                    current_state = "BLOCKED";
                    break;
                case "OPEN":
                    if (autoclose_on) {
                        lamp_command = cmd_light_blue;
                        node_status  = status_autoclose;
                    } else {
                        node_status   = status_open;
                        lamp_command  = cmd_light_green;
                        lamp_mode     = "cmd-then-off";
                    }
                    current_state = "OPEN";
                    break;
            }
        }
        else if(ctrl_topic == "autoclose" && msg.payload == "enabled") {
            context.set("autoclose_on", true);
        }
        break;

    case "CLOSED":
        node_status = status_closed;
        if(ctrl_topic == "current_hard_position"){
            if(msg.payload == "UNKNOWN"){
                node_status   = status_unknown;
                lamp_command  = cmd_light_red
                current_state = "UNKNOWN";
            }
        }
        else if(ctrl_topic == "autoclose" && msg.payload == "enabled") {
            context.set("autoclose_on", true);
        }
        else if(msg.topic == signal_light_topic && msg.event == "update"){
            lamp_command = cmd_light_red;
            lamp_mode    = "cmd-then-off";
        }
        break;

    case "OPEN":
        node_status = status_open;
        if(ctrl_topic == "current_hard_position"){
            switch(msg.payload) {
                case "UNKNOWN":
                    node_status   = status_unknown;
                    lamp_command  = cmd_light_red;
                    current_state = "UNKNOWN";
                    break;
                case "BLOCKED":
                    node_status   = status_blocked;
                    lamp_command  = cmd_light_yellow;
                    current_state = "BLOCKED";
                    break;
            }
        }
        else if(ctrl_topic == "autoclose" && msg.payload == "enabled") {
            context.set("autoclose_on", true);
            lamp_command = cmd_light_blue;
            node_status  = status_autoclose;
        }
        else if(ctrl_topic == "autoclose" && msg.payload == "reset") {
            context.set("autoclose_on", false);
            lamp_command = cmd_light_green;
            lamp_mode    = "cmd-then-off";
            node_status  = status_open;
        }
        else if(ctrl_topic == "autoclose" && msg.payload == "pending") {
            context.set("autoclose_on", true);
            lamp_command = cmd_light_blue;
            node_status  = status_autoclose;
        }
        else if(
            msg.topic == "CloseBtn" ||
            (ctrl_topic == "autoclose" && msg.payload == "triggered")
        ){
            gate_command  = "CLOSED";
            current_state = "CLOSE_WARN";
            context.set("autoclose_on", false);
        }
        else if(msg.topic == signal_light_topic && msg.event == "update"){
            lamp_command = cmd_light_green;
            lamp_mode    = "cmd-then-off";
        }
        break;

    case "BLOCKED":
        node_status = status_blocked;
        if (ctrl_topic == "current_hard_position") {
            switch (msg.payload) {
                case "OPEN":
                    if (autoclose_on) {
                        lamp_command = cmd_light_blue;
                        node_status  = status_autoclose;
                    } else {
                        node_status   = status_open;
                        lamp_command  = cmd_light_green;
                        lamp_mode     = "cmd-then-off";
                    }
                    current_state = "OPEN";
                    break;
                case "UNKNOWN":
                    node_status   = status_unknown;
                    lamp_command  = cmd_light_red;
                    current_state = "UNKNOWN";
                    break;
            }
        }
        else if(ctrl_topic == "autoclose" && msg.payload == "pending") {
            lamp_command  = cmd_light_blue;
            node_status   = status_autoclose;
            current_state = "OPEN";
        }
        break;

    case "CLOSE_WARN":
        if(ctrl_topic == "close_ack") {
            switch (msg.payload) {
                case "waiting":
                    lamp_command  = cmd_light_red;
                    lamp_mode     = "blink";
                    node_status   = status_closing;
                    break;

                case "reject":
                case "abort":
                    lamp_command  = cmd_light_green;
                    lamp_mode     = "cmd-then-off";
                    node_status   = status_open;
                    current_state = "OPEN";
                    break;

                case "commit":
                    lamp_command  = cmd_light_red;
                    node_status   = status_closing;
                    current_state = "UNKNOWN";
                    break;
            }
        }
        else if(ctrl_topic == "current_hard_position"){
            switch(msg.payload) {
                case "UNKNOWN":
                    node_status   = status_unknown;
                    lamp_command  = cmd_light_red;
                    current_state = "UNKNOWN";
                    break;
                case "BLOCKED":
                    node_status   = status_blocked;
                    lamp_command  = cmd_light_yellow;
                    current_state = "BLOCKED";
                    break;
            }
        }
        break;
}

context.set("state", current_state);

// Translate lamp_command into {payload: lamp_command} or null
var lamp_reset = null;
if (lamp_command !== null) {
    lamp_command = {payload: lamp_command};
    lamp_reset   = {reset: true};
}

// Translate gate_command into {payload: gate_command} or {reset: true}
if (gate_command === "reset" || current_state !== "CLOSE_WARN") {
    gate_command = {reset: true};
}
else if (gate_command === "CLOSED") {
    gate_command = {payload: gate_command};
}
else {
    gate_command = null;
}

if (node_status !== null) {
    node.status(node_status);
}

node.send([
    (lamp_mode == "direct"       ? lamp_command : null),
    (lamp_mode == "cmd-then-off" ? lamp_command : lamp_reset),
    (lamp_mode == "blink"        ? lamp_command : null),
    gate_command
]);

//   ----------------->8---------- CUT HERE ----------------------------------
}

module.exports = {step};
