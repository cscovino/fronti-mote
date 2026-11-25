# Spec Delta

## ADDED Requirements

### Requirement: RSSI signal bands
While online the pal SHALL map link RSSI to bands 1-4 at -55/-65/-72/-80
dBm thresholds, resampled about every 15 s, with about 4 dB hysteresis so
a reading near a threshold does not ping-pong; no link SHALL be band 0.
Band and link changes SHALL be reported to the panel's signal ladder.

#### Scenario: Steady signal
- **GIVEN** the link is online
- **WHEN** RSSI stays inside one band for minutes
- **THEN** the band never changes and the ladder never updates

#### Scenario: Threshold hover
- **WHEN** RSSI fluctuates within 4 dB of a band edge
- **THEN** the band stays on its side of the edge

#### Scenario: Link drops
- **WHEN** the link leaves online
- **THEN** the band is 0 at once, without waiting for a sample

#### Scenario: Link comes up
- **WHEN** the link goes online
- **THEN** the band is read immediately, not one sample period later

## MODIFIED Requirements

### Requirement: LED link indicator
The onboard LED (active LOW) SHALL be a binary lamp: solid on only while
online, off in every other state; the signal ladder on the panel carries
the graded link picture.

#### Scenario: Online
- **WHEN** the link is online
- **THEN** the LED is solid on

#### Scenario: Offline
- **GIVEN** the link is connecting, offline, rejected or unconfigured
- **WHEN** the pal is running
- **THEN** the LED is off
