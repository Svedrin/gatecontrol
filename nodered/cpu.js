function step(context, node, msg) {
//   ----------------->8---------- CUT HERE ----------------------------------
/**
 * Node outputs:
 *  1. LIFX Light (directly)
 *  2. LIFX Light via a trigger node that sends "off" 30 seconds after our command
 *  3. LIFX Light via a function node that lets it blink 5 times, then static on
 *     see blink-then-on.js
 *  4. Gate controller set_hard_position
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

var current_hard_position = null;
var node_status = null;

if (msg.topic == signal_light_topic) {
    // LIFX lights ack a change by sending msg.event="change"
    context.set("last_signal_light_message_recv_at", new Date());
}
else if (msg.topic == "ctrl/" + tor_esp + "/current_hard_position") {
    current_hard_position = msg.payload;
}

var last_signal_light_message_recv_at =
    new Date(context.get("last_signal_light_message_recv_at") || 0);

switch(context.get("state") || "UNKNOWN"){
    case "UNKNOWN":
        node_status = status_unknown;
        if(msg.topic == "ctrl/" + tor_esp + "/current_hard_position"){
            if(msg.payload == "CLOSED"){
                node_status = status_closed;
                node.send([null, {payload: cmd_light_red}, null, null]);
                context.set("state", "CLOSED");
            }
            else if(msg.payload == "OPEN"){
                node_status = status_open;
                node.send([null, {payload: cmd_light_green}, null, null]);
                context.set("state", "OPEN");
            }
        }
        node.status(node_status);
        break;

    case "CLOSED":
        node_status = status_closed;
        if(msg.topic == "ctrl/" + tor_esp + "/current_hard_position"){
            if(msg.payload == "UNKNOWN"){
                node_status = status_unknown;
                node.send([{payload: cmd_light_red}, {reset: true}, null, null]);
                context.set("state", "UNKNOWN");
            }
        } else if(msg.topic == "lifx/ampel" && msg.event == "update"){
            node.send([null, {payload: cmd_light_red}, null]);
        }
        node.status(node_status);
        break;

    case "OPEN":
        node_status = status_open;
        if(msg.topic == "ctrl/" + tor_esp + "/current_hard_position"){
            switch(msg.payload) {
                case "UNKNOWN":
                    node_status = status_unknown;
                    node.send([{payload: cmd_light_red}, {reset: true}, null, null]);
                    context.set("state", "UNKNOWN");
                    break;
                case "BLOCKED":
                    node_status = status_blocked;
                    node.send([{payload: cmd_light_yellow}, {reset: true}, null, null]);
                    context.set("state", "BLOCKED");
                    break;
            }
        }
        else if(
            msg.topic == "ctrl/" + tor_esp + "/autoclose" &&
            msg.payload == "pending"
        ) {
            node.send([{payload: cmd_light_blue}, {reset: true}, null, null]);
        }
        else if(
            msg.topic == "CloseBtn" ||
            (
                msg.topic == "ctrl/" + tor_esp + "/autoclose" &&
                msg.payload == "triggered"
            )
        ){
            context.set("state", "CLOSE_WARN");
            context.set("warn_start", new Date());

            node.send([null, null, {payload: cmd_light_red}, {payload: "CLOSED"}]);

            setTimeout(function(){
                if( context.get("state") == "CLOSE_WARN" ){
                    node.send([null, null, {payload: "COMMIT"}]);
                    context.set("state", "OPEN");
                }
            }, 10000);
        } else if(msg.topic == "lifx/ampel" && msg.event == "update"){
            node.send([null, {payload: cmd_light_green}, null, null]);
        }
        node.status(node_status);
        break;

    case "BLOCKED":
        if(
            msg.topic == "ctrl/" + tor_esp + "/current_hard_position" &&
            msg.payload == "OPEN"
        ){
            node.status(status_open);
            node.send([null, {payload: cmd_light_green}, null, null]);
            context.set("state", "OPEN");
        }
        else {
            node.send([{payload: cmd_light_yellow}, {reset: true}, null, null]);
        }
        break;
}

//   ----------------->8---------- CUT HERE ----------------------------------
}

module.exports = {step};
