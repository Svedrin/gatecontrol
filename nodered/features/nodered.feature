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
