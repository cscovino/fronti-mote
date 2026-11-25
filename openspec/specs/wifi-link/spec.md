# wifi-link Specification

## Purpose

Joining, keeping and signalling the Wi-Fi station link via a binary lamp,
RSSI signal bands, tunes and scan.

## Requirements

### Requirement: Station-mode connect
The pal SHALL join one Wi-Fi network in station mode using credentials compiled from a gitignored settings file; with no file or an empty SSID the radio stays off and everything else keeps working. The address comes from DHCP and the hostname is `frontimote-` plus the last six hex digits of the MAC.

#### Scenario: No credentials
- **GIVEN** no credentials are configured
- **WHEN** the pal boots
- **THEN** Wi-Fi stays off, the console says so, and the face, knob and buzzer still work

### Requirement: Reconnect and retry
A link that drops SHALL reconnect by itself. A rejected password SHALL be retried no faster than every 10 s, and a brief reconnect attempt that succeeds SHALL not be reported as a drop (3 s debounce).

#### Scenario: Brief dropout
- **GIVEN** the link is online
- **WHEN** the connection drops and returns within 3 s
- **THEN** the link status never leaves online

#### Scenario: Wrong password
- **GIVEN** the configured password is rejected
- **WHEN** joins are attempted
- **THEN** each attempt is at least 10 s after the previous one

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

### Requirement: Network scan
At boot the firmware SHALL scan once (blocking a few seconds inside setup), and the console `wifi` command SHALL start the same scan in the background and print the results a few seconds later without freezing the pal.

#### Scenario: Scan from console
- **WHEN** `wifi` is typed
- **THEN** the current link state prints at once and scan results follow seconds later while the knob stays responsive

### Requirement: Link tunes
Bringing the link up SHALL play the rising tune and losing a link that was up SHALL play the falling tune, as specified by the buzzer capability.

#### Scenario: Link comes up
- **WHEN** Wi-Fi associates and gets an address
- **THEN** the up tune plays once
