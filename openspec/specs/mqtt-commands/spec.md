# mqtt-commands Specification

## Purpose

Remote control over an MQTT broker: command topics, state publishing and last will.

## Requirements

### Requirement: Broker connection lifecycle
With a broker host configured, the pal SHALL connect over plain TCP (no TLS) once Wi-Fi is online and retry every 5 s. An empty host leaves MQTT off entirely.

#### Scenario: No broker configured
- **GIVEN** the broker host is empty
- **WHEN** the pal runs
- **THEN** no MQTT connection is attempted and everything else works

### Requirement: Command interface
The pal SHALL execute JSON commands received on `<prefix>/cmd`: `{"face": N}` sets a face by name (case-insensitive) or `next` cycles; `{"beep": HZ}` plays one 200 ms note for 100–5000 Hz; `{"tune": N}` plays a named tune; `{"status": true}` publishes the state immediately.

#### Scenario: Face by name
- **WHEN** `{"face":"happy"}` is published
- **THEN** the pal shows the Happy face

#### Scenario: Out-of-range beep
- **WHEN** `{"beep":50}` is published
- **THEN** no note plays and nothing else changes

### Requirement: Bad payloads ignored
Malformed JSON, payloads with no recognized command, and payloads exceeding the 512-byte receive buffer SHALL be ignored; the first two are logged on the serial console, oversized ones are dropped silently.

#### Scenario: Garbage payload
- **WHEN** `not json at all` is published to the command topic
- **THEN** the pal logs it on the console and its state is unchanged

### Requirement: State topic
The pal SHALL publish its state — online flag, current face, RSSI, free heap, uptime — as retained JSON on `<prefix>/state` on connect and on every face change, and register `{"online":false}` as its last will.

#### Scenario: Face change publishes state
- **WHEN** the face changes by any input (knob, console, MQTT)
- **THEN** the new state appears retained on the state topic

#### Scenario: Crash reported
- **GIVEN** the pal was connected
- **WHEN** it loses power
- **THEN** subscribers receive the last will `{"online":false}`

### Requirement: Multi-unit isolation
Two units sharing one broker SHALL be independent when configured with different topic prefixes; the prefix is the only isolation mechanism.

#### Scenario: Shared prefix warning
- **GIVEN** two units on one broker share a prefix
- **WHEN** a command is published
- **THEN** both units react and both publish to the same state topic (documented behavior, not a bug)
