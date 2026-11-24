# serial-console Specification

## Purpose

The interactive serial console: commands, line handling, heartbeat and boot banner.

## Requirements

### Requirement: Console commands
The pal SHALL accept single-word commands on its serial console (115200, native USB CDC), one per line, case-sensitive lowercase: `face` cycles to the next expression, `buzz` plays a 440 Hz test note, `wifi` prints the link state and starts a background scan, `status` prints chip/flash/heap/face/Wi-Fi/MQTT information, `help` lists the commands. An unrecognized word gets an error line pointing at `help`.

#### Scenario: Face command
- **WHEN** `face` is typed and entered
- **THEN** the pal cycles to the next expression

#### Scenario: Unknown command
- **WHEN** `frobnicate` is typed and entered
- **THEN** the console prints `Unknown command: frobnicate (try 'help')`

### Requirement: Line editing and limits
The console SHALL echo typed characters, accept CR, LF or CRLF as line end, honor backspace, ignore other control bytes, and reject lines over 63 characters with a message without executing them.

#### Scenario: Overlong paste
- **WHEN** a line longer than 63 characters is sent
- **THEN** the console prints `Line too long (max 63 chars)` and runs no command

### Requirement: Heartbeat and boot banner
The console SHALL print a startup banner followed by status and help once after boot, and a one-line heartbeat with uptime and free heap every 2 s.

#### Scenario: Boot
- **WHEN** the pal starts with a host attached
- **THEN** the banner, a status block and the help text print in order

### Requirement: Console never blocks the pal
Serial writes SHALL never stall `loop()` on an unresponsive USB host (log lines are dropped instead), and input handling SHALL process a bounded burst per loop so a paste cannot starve the rest of the firmware.

#### Scenario: No USB host
- **GIVEN** nothing is reading the serial port
- **WHEN** the heartbeat keeps logging
- **THEN** the knob, buzzer and panel keep working
