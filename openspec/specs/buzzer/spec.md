# buzzer Specification

## Purpose

The tones and tunes the pal sounds on its passive buzzer, including the link up and down tunes.

## Requirements

### Requirement: Tone generation
The pal SHALL sound a passive buzzer by generating the tone itself; a note is a fixed pitch for a fixed duration, and silence means no tone is driven.

#### Scenario: Single note
- **WHEN** the pal is asked to play one 440 Hz note
- **THEN** the buzzer sounds 440 Hz for 200 ms and then goes silent

### Requirement: Tune playback
The pal SHALL play named tunes, each an ordered sequence of notes with silent gaps between them. The tunes and their note sequences are: `up` (523, 659, 784 Hz rising), `down` (392, 294 Hz falling), `happy`, `sad`, `curious` and `sleepy` mood tunes.

#### Scenario: Play a mood tune
- **WHEN** the tune `happy` is requested
- **THEN** four rising notes play with gaps between them and the buzzer is silent after the last

#### Scenario: Tune names are case-insensitive
- **WHEN** a request names the tune `UP`
- **THEN** the up tune plays

### Requirement: Link tunes
A link that comes up SHALL play the three-note rising `up` tune and a link that was up and goes down SHALL play the two-note falling `down` tune. A link that never came up SHALL stay silent across its retries.

#### Scenario: Wi-Fi connects
- **WHEN** the Wi-Fi link transitions to online
- **THEN** the up tune plays

#### Scenario: Repeated failed reconnects
- **GIVEN** the link has never connected since boot
- **WHEN** joins are retried
- **THEN** no tune plays

### Requirement: One tune at a time
Starting a new note or tune while one is playing SHALL replace the ongoing sequence, not overlap it.

#### Scenario: Beep during a tune
- **GIVEN** a tune is mid-sequence
- **WHEN** a single beep is requested
- **THEN** the beep plays and the rest of the tune is abandoned
