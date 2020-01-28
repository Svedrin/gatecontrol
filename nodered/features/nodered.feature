Feature: NodeRED

  Scenario: easy states
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

  Scenario: Autoclose pending from "blocked" state, runs to completion
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

    When the gate accepts the COMMIT command
    Then the signal light is switched to red permanently
    And the node status is "red" and says "closing"

    When the gate is moving
    Then no commands are sent

    When the gate is closed
    Then the signal light is switched to red, then off
    And the node status is "green" and says "closed"

  Scenario: Autoclose pending from "blocked" state, gets blocked
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

  Scenario: Autoclose pending from "open" state, gets reset
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

  Scenario: CLOSED command gets rejected
    When the gate is open
     And the Close button in the GUI is pressed
    Then the gate receives a CLOSED command

    When the gate rejects the CLOSED command
    Then the signal light is switched to green, then off
    And the node status is "red" and says "open"

  Scenario: CLOSED command gets accepted
    When the gate is open
     And the Close button in the GUI is pressed
    Then the gate receives a CLOSED command

    When the gate accepts the CLOSED command
    Then the signal light blinks red
    And the node status is "red" and says "closing"
