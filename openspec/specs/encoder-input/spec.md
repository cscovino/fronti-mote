# encoder-input Specification

## Purpose

Decoding the rotary encoder into clean detent steps that drive face changes.

## Requirements

### Requirement: Detent decoding
The firmware SHALL decode the rotary encoder's A/B quadrature signals into discrete detent steps of −1, 0 or +1, emitting one step per physical click.

#### Scenario: One click forward
- **WHEN** the encoder is turned one detent clockwise
- **THEN** exactly one +1 step is reported

#### Scenario: Idle encoder reports nothing
- **WHEN** the encoder is not moved
- **THEN** no step is reported

### Requirement: Bounce rejection
Contacts bouncing, or both A and B changing in the same sample, SHALL NOT produce a phantom detent; invalid transitions cancel out.

#### Scenario: Jitter at a detent
- **GIVEN** the encoder contacts chatter at a detent edge
- **WHEN** the quadrature sequence is completed or reversed
- **THEN** at most one net step is reported for the real movement

### Requirement: Detent drives the pal
Each decoded detent SHALL cycle the face one step in the turn's direction and sound that face's note immediately, without waiting for the panel refresh.

#### Scenario: Spin the knob
- **WHEN** the user turns three detents while the pal shows Calm
- **THEN** the notes for Happy, Surprised and Sleepy play in order and the pending face is Sleepy
