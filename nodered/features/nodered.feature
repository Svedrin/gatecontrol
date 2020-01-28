Feature: NodeRED

  Scenario: easy states

    Let's run a quick test for when the gate is opened and closed manually,
    without anything fancy happening in between.

    When the gate is closed
    Then the signal light is switched to red, then off
    And the node status is "green" and says "closed"

    When the gate is moving
    Then the signal light is switched to red permanently
    And the node status is "yellow" and says "unknown"

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When the gate is blocked
    Then the signal light is switched to yellow permanently
    And the node status is "yellow" and says "blocked"

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When the gate is moving
    Then the signal light is switched to red permanently
    And the node status is "yellow" and says "unknown"

    When the gate is closed
    Then the signal light is switched to red, then off
    And the node status is "green" and says "closed"

  Scenario: open -> blocked -> unknown -> closed

    Sometimes I push the remote when leaving the garage while the car
    is still blocking the gate, so that OPEN will be skipped and we
    go directly from BLOCKED to UNKNOWN. Make sure this works.

    When the gate is closed
    Then the signal light is switched to red, then off
    And the node status is "green" and says "closed"

    When the gate is moving
    Then the signal light is switched to red permanently
    And the node status is "yellow" and says "unknown"

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When the gate is blocked
    Then the signal light is switched to yellow permanently
    And the node status is "yellow" and says "blocked"

    When the gate is moving
    Then the signal light is switched to red permanently
    And the node status is "yellow" and says "unknown"

    When the gate is closed
    Then the signal light is switched to red, then off
    And the node status is "green" and says "closed"

  Scenario: Autoclose pending from "blocked" state, runs to completion

    Enable autoclose, go to BLOCKED, and then send autoclose=pending
    without going through the OPEN state first. Then simulate the
    close procedure as it is meant to be. This test also checks what
    happens if the gate is blocked multiple times.

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When autoclose has been enabled
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When the gate is blocked
    Then the signal light is switched to yellow permanently
    And the node status is "yellow" and says "blocked"

    When autoclose is pending
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When the gate is open
    Then no commands are sent

    When the gate is blocked
    Then the signal light is switched to yellow permanently
    And the node status is "yellow" and says "blocked"

    When autoclose is pending
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When the gate is open
    Then no commands are sent

    When autoclose has triggered
    Then the gate receives a CLOSED command

    When the gate accepts the CLOSED command
    Then the signal light blinks red
    And the node status is "red" and says "closing"

    When the gate accepts the COMMIT command
    Then the signal light is switched to red permanently
    And the node status is "red" and says "closing"

    When the gate is moving
    Then no commands are sent

    When the gate is closed
    Then the signal light is switched to red, then off
    And the node status is "green" and says "closed"

  Scenario: Autoclose pending from "blocked" state, gets blocked

    Enable autoclose, go to BLOCKED, then back to OPEN, _then_
    send autoclose=pending. Basically the other way around as
    above, because we cannot know which one is going to happen
    first. Then see what happens when the gate gets blocked
    before the close procedure gets to the COMMIT command.

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When autoclose has been enabled
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When the gate is blocked
    Then the signal light is switched to yellow permanently
    And the node status is "yellow" and says "blocked"

    When autoclose is pending
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose has triggered
    Then the gate receives a CLOSED command

    When the gate accepts the CLOSED command
    Then the signal light blinks red
    And the node status is "red" and says "closing"

    When the gate is blocked
    Then the signal light is switched to yellow permanently
    And the node status is "yellow" and says "blocked"

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

  Scenario: Autoclose pending from "open" state, gets aborted

    See what happens when autoclose gets to the point where
    the COMMIT command is sent, but the COMMIT gets rejected
    by the ESP.

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When autoclose has been enabled
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When the gate is blocked
    Then the signal light is switched to yellow permanently
    And the node status is "yellow" and says "blocked"

    When the gate is open
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose is pending
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose has triggered
    Then the gate receives a CLOSED command

    When the gate accepts the CLOSED command
    Then the signal light blinks red
    And the node status is "red" and says "closing"

    When the gate rejects the COMMIT command
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

  Scenario: Autoclose pending from "blocked" state, user closes gate manually

    Autoclose runs normally, then triggers, but before we send a COMMIT,
    somebody uses the gate control button to close the gate manually.
    Make sure we cancel our commit and update the lamp correctly.

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When autoclose has been enabled
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When the gate is blocked
    Then the signal light is switched to yellow permanently
    And the node status is "yellow" and says "blocked"

    When autoclose is pending
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose has triggered
    Then the gate receives a CLOSED command

    When the gate accepts the CLOSED command
    Then the signal light blinks red
    And the node status is "red" and says "closing"

    When the gate is moving
    Then the signal light is switched to red permanently
    And the node status is "yellow" and says "unknown"
    And the gate command is reset

    When the gate is closed
    Then the signal light is switched to red, then off
    And the node status is "green" and says "closed"

  Scenario: Autoclose pending from "open" state, gets reset

    See what happens when autoclose reaches the pending state,
    but then gets reset.

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When autoclose has been enabled
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose is pending
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose gets reset
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

  Scenario: Autoclose timeout from "open" state, gets reset

    Set autoclose enabled, and sit around in OPEN until autoclose
    times out. Then check that we react accordingly.

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When autoclose has been enabled
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose gets reset
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

  Scenario: Autoclose pending from "unknown" state

    Check that when autoclose is enabled while the gate is in
    unknown state, this fact gets preserved and when the gate
    eventually reaches OPEN, the light gets switched to blue.

    When the gate is closed
    Then the signal light is switched to red, then off
    And the node status is "green" and says "closed"

    When the gate is moving
    Then the signal light is switched to red permanently
    And the node status is "yellow" and says "unknown"

    When autoclose has been enabled
    Then no commands are sent

    When the gate is open
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose is pending
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose gets reset
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

  Scenario: Autoclose pending from "closed" state

    Check that when autoclose is enabled while the gate is in
    closed state, this fact gets preserved and when the gate
    eventually reaches OPEN, the light gets switched to blue.

    When the gate is closed
    Then the signal light is switched to red, then off
    And the node status is "green" and says "closed"

    When autoclose has been enabled
    Then no commands are sent

    When the gate is moving
    Then the signal light is switched to red permanently
    And the node status is "yellow" and says "unknown"

    When the gate is open
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose is pending
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose gets reset
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

  Scenario: Autoclose pending from "open" state, user closes gate

    See what happens when autoclose reaches the pending state,
    but then the user manually closes the gate.

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When autoclose has been enabled
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose is pending
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When the gate is moving
    Then the signal light is switched to red permanently
    And the node status is "yellow" and says "unknown"

  Scenario: Autoclose pending from "open" state, user moves gate

    See what happens when autoclose reaches the pending state,
    but then the user manually closes the gate, then changes
    their mind and moves it back to open. The autoclose status
    should have been reset on our side by now, so the lamp
    does not switch to blue, but instead to green.

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

    When autoclose has been enabled
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When autoclose is pending
    Then the signal light is switched to blue permanently
    And the node status is "blue" and says "open+autoclose"

    When the gate is moving
    Then the signal light is switched to red permanently
    And the node status is "yellow" and says "unknown"

    When the gate is open
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

  Scenario: CLOSED command gets rejected

    Let's see what happens when a CLOSED command is rejected by the ESP.

    When the gate is open
     And the Close button in the GUI is pressed
    Then the gate receives a CLOSED command

    When the gate rejects the CLOSED command
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

  Scenario: CLOSED command gets accepted

    Let's see what happens when a CLOSED command is accepted by the ESP.

    When the gate is open
     And the Close button in the GUI is pressed
    Then the gate receives a CLOSED command

    When the gate accepts the CLOSED command
    Then the signal light blinks red
    And the node status is "red" and says "closing"
