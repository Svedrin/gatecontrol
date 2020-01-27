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
    return msg;
}
else if (msg.payload == "COMMIT") {
    var last_update =
        new Date(context.get("last_update") || 0);
    if (new Date() - last_update < 10000) {
        return msg;
    }
}
