Feature: Simple maths
  In order to do maths
  As a developer
  I want to increment variables

  Scenario: easy maths
    When topic "ctrl/a1234b/current_hard_position" receives a message of "CLOSED"
    Then the signal light is switched to red, then off
