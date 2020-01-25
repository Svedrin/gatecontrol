const { Given, When, Then } = require("cucumber");
const { assert } = require("chai");

When('topic {string} receives a message of {string}', function(topic, payload) {
    this.step(this.node_ctx, this.node, {topic, payload});
});

Then("the signal light is switched to red, then off", function() {
    assert.deepEqual(this.node.last_messages,
        [null, {payload: this.cmd_light_red}, "not null]
    );
});
