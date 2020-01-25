function step(context, node, msg) {
//   ----------------->8---------- CUT HERE ----------------------------------
/**
 * Node outputs:
 *  1. Light (direct)
 *  2. Light (state X, 30s later: off)
 *  3. Gate controller set_hard_position
 */

var tor_esp = "a1234b";
var cmd_light_red    = { on: true, hex: "#DC3545", duration: 100, bri: 60 };
var cmd_light_yellow = { on: true, hex: "#efef1a", duration: 100, bri: 60 };
var cmd_light_green  = { on: true, hex: "#28A745", duration: 100, bri: 60 };
var cmd_light_blue   = { on: true, hex: "#32d2ff", duration: 100, bri: 60 };
var status_open      = { fill: "red",    shape: "dot", text: "open"    };
var status_unknown   = { fill: "yellow", shape: "dot", text: "unknown" };
var status_closed    = { fill: "green",  shape: "dot", text: "closed"  };
var status_blocked   = { fill: "yellow", shape: "dot", text: "blocked" };

switch(context.get("state") || "UNKNOWN"){
    case "UNKNOWN":
        node.status(status_unknown);
        if(msg.topic == "ctrl/" + tor_esp + "/current_hard_position"){
            if(msg.payload == "CLOSED"){
                node.status(status_closed);
                node.send([null, {payload: cmd_light_red}, null]);
                context.set("state", "CLOSED");
            }
            else if(msg.payload == "OPEN"){
                node.status(status_open);
                node.send([null, {payload: cmd_light_green}, null]);
                context.set("state", "OPEN");
            }
        }
        break;

    case "CLOSED":
        node.status(status_closed);
        if(msg.topic == "ctrl/" + tor_esp + "/current_hard_position"){
            if(msg.payload == "UNKNOWN"){
                node.status(status_unknown);
                node.send([{payload: cmd_light_red}, {reset: true}, null]);
                context.set("state", "UNKNOWN");
            }
        } else if(msg.topic == "lifx/ampel" && msg.event == "update"){
            node.send([null, {payload: cmd_light_red}, null]);
        }
        break;

    case "OPEN":
        node.status(status_open);
        if(msg.topic == "ctrl/" + tor_esp + "/current_hard_position"){
            switch(msg.payload) {
                case "UNKNOWN":
                    node.status(status_unknown);
                    node.send([{payload: cmd_light_red}, {reset: true}, null]);
                    context.set("state", "UNKNOWN");
                    break;
                case "BLOCKED":
                    node.status(status_blocked);
                    node.send([{payload: cmd_light_yellow}, {reset: true}, null]);
                    context.set("state", "BLOCKED");
                    break;
            }
        }
        else if(
            msg.topic == "ctrl/" + tor_esp + "/autoclose" &&
            msg.payload == "pending"
        ) {
            node.send([{payload: cmd_light_blue}, {reset: true}, null]);
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

            var warn_lamp = function(lamp_state){
                var cmd_light_red_copy = Object.assign({}, cmd_light_red);
                cmd_light_red_copy.on = lamp_state;
                node.send([
                    {payload: cmd_light_red_copy},
                    {reset: true},
                    null
                ]);
                if( new Date() - context.get("warn_start") < 5000 ){
                    setTimeout(warn_lamp, 500, !lamp_state);
                }
            }

            warn_lamp(true);

            node.send([null, null, {payload: "CLOSED"}]);

            setTimeout(function(){
                if( context.get("state") == "CLOSE_WARN" ){
                    node.send([null, null, {payload: "COMMIT"}]);
                    context.set("state", "OPEN");
                }
            }, 10000);
        } else if(msg.topic == "lifx/ampel" && msg.event == "update"){
            node.send([null, {payload: cmd_light_green}, null]);
        }
        break;

    case "BLOCKED":
        node.send([{payload: cmd_light_yellow}, {reset: true}, null]);
        if(
            msg.topic == "ctrl/" + tor_esp + "/current_hard_position" &&
            msg.payload == "OPEN"
        ){
            node.status(status_open);
            node.send([null, {payload: cmd_light_green}, null]);
            context.set("state", "OPEN");
        }
        break;
}

//   ----------------->8---------- CUT HERE ----------------------------------
}

module.exports = {step};
