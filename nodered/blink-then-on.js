// Code for the blink-then-on node, which is attached to the CPU
// on output 3.

var warn_lamp = function(
    lamp_state, orig_msg, warn_start
){
    var new_msg = Object.assign({}, orig_msg);
    new_msg.on = lamp_state;
    node.send({
        payload: new_msg
    });
    if (new Date() - warn_start < 5000) {
        setTimeout(
            warn_lamp,
            500,
            !lamp_state,
            orig_msg,
            warn_start
        );
    }
}

warn_lamp(true, msg.payload, new Date());
