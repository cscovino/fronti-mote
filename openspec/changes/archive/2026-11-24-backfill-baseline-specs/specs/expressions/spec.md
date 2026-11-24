# Spec Delta: expressions

## ADDED Requirements

### Requirement: Face set and order
The pal SHALL support exactly four expressions — sleepy, calm, happy, surprised — displayed in that fixed order, with calm as the face shown at boot.

#### Scenario: Boot face
- **WHEN** the firmware finishes starting
- **THEN** the panel shows the Calm face

#### Scenario: Face names are stable
- **WHEN** any component reports a face by name (serial, MQTT, state topic)
- **THEN** the names are `sleepy`, `calm`, `happy`, `surprised` in lowercase

### Requirement: Wrap-around cycling
Cycling the face SHALL move through the four expressions in a ring in both directions, so the step past either end wraps to the other end.

#### Scenario: Forward past the last face
- **GIVEN** the pal shows Surprised
- **WHEN** the face advances one step
- **THEN** the pal shows Sleepy

#### Scenario: Backward past the first face
- **GIVEN** the pal shows Sleepy
- **WHEN** the face moves one step back
- **THEN** the pal shows Surprised

### Requirement: Per-face note
Each expression SHALL map to one fixed pitch: sleepy 262 Hz, calm 330 Hz, happy 392 Hz, surprised 523 Hz.

#### Scenario: Landing on a face plays its note
- **WHEN** the face changes to Happy
- **THEN** the buzzer sounds one 392 Hz note

### Requirement: Face lookup by name
A face SHALL be selectable by name, case-insensitively; an unrecognized name leaves the current face unchanged.

#### Scenario: Case-insensitive match
- **WHEN** a request names the face `HaPpY`
- **THEN** the pal shows the Happy face

#### Scenario: Unknown name ignored
- **WHEN** a request names the face `angry`
- **THEN** the pal keeps its current face
