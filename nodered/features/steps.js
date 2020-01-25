const { Given, When, Then } = require("cucumber");
const { assert } = require("chai");

When('the gate is closed', function() {
    this.step(this.node_ctx, this.node, {
        topic:   "ctrl/a1234b/current_hard_position",
        payload: "CLOSED"
    });
});

When('the gate is moving', function() {
    this.step(this.node_ctx, this.node, {
        topic:   "ctrl/a1234b/current_hard_position",
        payload: "UNKNOWN"
    });
});

When('the gate is open', function() {
    this.step(this.node_ctx, this.node, {
        topic:   "ctrl/a1234b/current_hard_position",
        payload: "OPEN"
    });
});

When('the gate is blocked', function() {
    this.step(this.node_ctx, this.node, {
        topic:   "ctrl/a1234b/current_hard_position",
        payload: "BLOCKED"
    });
});

Then("the signal light is switched to red, then off", function() {
    assert.deepEqual(this.node.last_messages,
        [null, {payload: this.cmd_light_red}, null]
    );
});

Then("the signal light is switched to red permanently", function() {
    assert.deepEqual(this.node.last_messages,
        [{payload: this.cmd_light_red}, {reset: true}, null]
    );
});

Then("the signal light is switched to yellow permanently", function() {
    assert.deepEqual(this.node.last_messages,
        [{payload: this.cmd_light_yellow}, {reset: true}, null]
    );
});

Then("the signal light is switched to green, then off", function() {
    assert.deepEqual(this.node.last_messages,
        [null, {payload: this.cmd_light_green}, null]
    );
});

Then("the node status is {string} and says {string}", function(color, text) {
    assert.deepEqual(this.node.last_status,
        {fill: color, shape: "dot", text: text}
    );
});
