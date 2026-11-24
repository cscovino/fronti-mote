# Spec Delta: epaper-display

## ADDED Requirements

### Requirement: Face rendering
The pal SHALL draw the current expression on a 200×200 red/black/white e-paper panel: head, ears, spectacles and nose in black, with the face-specific eyes and mouth; red appears only as Happy's tongue.

#### Scenario: Happy face
- **WHEN** the Happy face is drawn
- **THEN** the panel shows arc eyes, an open smile and a red tongue

#### Scenario: Other faces have no red
- **WHEN** any face other than Happy is drawn
- **THEN** the picture contains only black and white pixels

### Requirement: Full refresh only
Every panel update SHALL be a full refresh (the screen flashes black and white); the panel has no usable partial waveform.

#### Scenario: Face change flashes
- **WHEN** the displayed face changes
- **THEN** the panel performs one full refresh showing the new face

### Requirement: Refresh pacing
The panel SHALL not refresh: closer than 2 s after the previous refresh, or when the face to show equals the face already displayed. A face change lands about 400 ms after the last encoder detent, so a continuous spin costs one refresh.

#### Scenario: Unchanged face skipped
- **GIVEN** the panel shows Calm
- **WHEN** the face is set to Calm again
- **THEN** no refresh happens

#### Scenario: Spin costs one update
- **GIVEN** the last refresh is more than 2 s ago
- **WHEN** the knob is turned five detents in one second
- **THEN** exactly one refresh shows the final face

#### Scenario: Within the gap
- **GIVEN** a refresh completed less than 2 s ago
- **WHEN** the face changes
- **THEN** the refresh is delayed until the 2 s gap has passed

### Requirement: Refresh never blocks input
A panel refresh SHALL run outside the main loop so the encoder and buzzer keep responding while the screen flashes.

#### Scenario: Turning during a refresh
- **GIVEN** a refresh is in progress
- **WHEN** the knob is turned
- **THEN** notes play immediately for each detent
