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

var tor_esp = "a1234b";
var signal_light_topic = "lifx/ampel";

var cmd_light_red    = { on: true, hex: "#DC3545", duration: 100, bri: 60 };
var cmd_light_yellow = { on: true, hex: "#efef1a", duration: 100, bri: 60 };
var cmd_light_green  = { on: true, hex: "#28A745", duration: 100, bri: 60 };
var cmd_light_blue   = { on: true, hex: "#32d2ff", duration: 100, bri: 60 };
var status_open      = { fill: "red",    shape: "dot", text: "open"    };
var status_unknown   = { fill: "yellow", shape: "dot", text: "unknown" };
var status_closed    = { fill: "green",  shape: "dot", text: "closed"  };
var status_blocked   = { fill: "yellow", shape: "dot", text: "blocked" };

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

switch (current_state) {
    case "UNKNOWN":
        node_status = status_unknown;
        if (ctrl_topic == "current_hard_position") {
            if(msg.payload == "CLOSED"){
                node_status   = status_closed;
                lamp_command  = cmd_light_red;
                lamp_mode     = "cmd-then-off";
                current_state = "CLOSED";
            }
            else if(msg.payload == "OPEN"){
                node_status   = status_open;
                lamp_command  = cmd_light_green;
                lamp_mode     = "cmd-then-off";
                current_state = "OPEN";
            }
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
        } else if(msg.topic == signal_light_topic && msg.event == "update"){
            lamp_command = cmd_light_red;
            lamp_mode    = "cmd-then-off";
        }
        break;

    case "OPEN":
        node_status = status_open;
        if(ctrl_topic ==  "current_hard_position"){
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
        else if(ctrl_topic == "autoclose" && msg.payload == "pending") {
            lamp_command = cmd_light_blue;
        }
        else if(
            msg.topic == "CloseBtn" ||
            (ctrl_topic == "/autoclose" && msg.payload == "triggered")
        ){
            lamp_command  = cmd_light_red;
            lamp_mode     = "blink";
            gate_command  = "CLOSED";
            current_state = "CLOSE_WARN";
        } else if(msg.topic == signal_light_topic && msg.event == "update"){
            lamp_command = cmd_light_green;
            lamp_mode    = "cmd-then-off";
        }
        break;

    case "BLOCKED":
        node_status = status_blocked;
        if (ctrl_topic == "current_hard_position" && msg.payload == "OPEN") {
            lamp_command  = cmd_light_green;
            lamp_mode     = "cmd-then-off";
            node_status   = status_open;
            current_state = "OPEN";
        }
        else {
            lamp_command = cmd_light_yellow;
            lamp_mode    = "direct";
        }
        break;
}

context.set("state", current_state);

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
    (lamp_mode == "direct"       ? {payload: lamp_command} : null),
    (lamp_mode == "cmd-then-off" ? {payload: lamp_command} : {reset: (lamp_command !== null)}),
    (lamp_mode == "blink"        ? {payload: lamp_command} : null),
    gate_command
]);

//   ----------------->8---------- CUT HERE ----------------------------------
}

module.exports = {step};
