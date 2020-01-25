
function step(context, node, msg) {
//   ----------------->8---------- CUT HERE ----------------------------------
/**
 * State machine:
 *
 *                  UNKNOWN                      CLOSED
 *                  ->Lamp Red                 ->Lamp Red -> Off 30s
 *         OPEN  -------------->   UNKNOWN   ----------->  CLOSED
 *           |  <--------------       ^     <-----------
 *           |      OPEN              |         UNKNOWN
 *           |      ->Lamp green,     |        ->Lamp Red
 *           |        off 30s         |
 *           |                        |
 *  CloseBtn |                        | UNKNOWN
 *           v            10s         |
 *         CLOSE_WARN  -------->  CLOSE_TRGR
 *         ->CLOSED               ->COMMIT
 *         ->Lamp blink red 5x,
 *           then static
 *
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

switch(context.get("state") || "UNKNOWN"){
    case "UNKNOWN":
        if(msg.topic == "ctrl/" + tor_esp + "/current_hard_position"){
            if(msg.payload == "CLOSED"){
                node.send([null, {payload: cmd_light_red}, null]);
                context.set("state", "CLOSED");
            }
            else if(msg.payload == "OPEN"){
                node.send([null, {payload: cmd_light_green}, null]);
                context.set("state", "OPEN");
            }
        }
        node.status({fill:"yellow", shape:"dot", text:"unknown"});
        break;

    case "CLOSED":
        if(msg.topic == "ctrl/" + tor_esp + "/current_hard_position"){
            if(msg.payload == "UNKNOWN"){
                node.send([{payload: cmd_light_red}, {reset: true}, null]);
                context.set("state", "UNKNOWN");
            }
        } else if(msg.topic == "lifx/ampel" && msg.event == "update"){
            node.send([null, {payload: cmd_light_red}, null]);
        }
        node.status({fill:"green", shape:"dot", text:"closed"});
        break;

    case "OPEN":
        if(msg.topic == "ctrl/" + tor_esp + "/current_hard_position"){
            switch(msg.payload) {
                case "UNKNOWN":
                    node.send([{payload: cmd_light_red}, {reset: true}, null]);
                    context.set("state", "UNKNOWN");
                    break;
                case "BLOCKED":
                    node.send([{payload: cmd_light_yellow}, {reset: true}, null]);
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
                node.send([
                    {payload: { on: lamp_state, hex: cmd_light_red.hex, duration: 10, bri: 60 }},
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
        node.status({fill:"red", shape:"dot", text:"open"});
        break;
}

//   ----------------->8---------- CUT HERE ----------------------------------
}

module.exports = {step};
