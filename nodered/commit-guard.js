// Code for the commit guard, which sits between a trigger node attached
// to the CPU on output 4, and the MQTT sender connected to set_hard_position.
//
// If topic is "lifx/ampel", note time
// If payload is "CLOSED", pass it through
// If payload is "COMMIT", check for last update
//    and only let the message through if that
//    was less than ten seconds ago
// Everything else is ignored

if (msg.topic == "lifx/ampel") {
    context.set("last_update", new Date());
}
else if (msg.payload == "CLOSED") {
    context.set("last_update", 0);
    node.status({ fill: "yellow", shape: "dot", text: "CLOSED" });
    return msg;
}
else if (msg.payload == "COMMIT") {
    var last_update =
        new Date(context.get("last_update") || 0);
    if (new Date() - last_update < 10000) {
        node.status({ fill: "green", shape: "dot", text: "COMMIT" });
        return msg;
    } else {
        node.status({ fill: "red", shape: "dot", text: "BLOCKED" });
        node.warn(
            "COMMIT but haven't heard back " +
            "from the signal light, blocking"
        );
    }
}
