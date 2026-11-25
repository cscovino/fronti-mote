# Spec Delta

## ADDED Requirements

### Requirement: Signal ladder
The panel SHALL reserve a ladder box in its free top-right corner
(x 160-199, y 4-27) showing the current signal band 1-4 as bars of
rising height, band 0 as an empty box; the ladder belongs to every face
picture and every ladder-only update.

#### Scenario: Strong signal
- **WHEN** a refresh happens with band 4
- **THEN** all four bars are filled

#### Scenario: No link
- **WHEN** a refresh happens with band 0
- **THEN** the box is empty

### Requirement: Fast ladder updates
A change of the ladder alone SHALL use the panel's fast partial
waveform, confined to the ladder box, with no screen-wide flash; a face
change SHALL keep using the flashing full refresh that draws face and
ladder together.

#### Scenario: Band moves quietly
- **GIVEN** a face is displayed
- **WHEN** only the signal band changes
- **THEN** only the ladder box visibly changes, in about half a second

#### Scenario: Face change draws everything
- **WHEN** the face changes
- **THEN** one full refresh shows the new face with the current ladder

### Requirement: Periodic full clears
No more than 500 consecutive ladder-only partial updates SHALL happen
without a full refresh clearing ghosting; a face refresh resets the
count. (Verified on the unit: 400 consecutive partials left the test
card clean.)

#### Scenario: Ghost budget spent
- **GIVEN** 500 ladder updates since the last full refresh
- **WHEN** the next ladder update is due
- **THEN** it is a full refresh

## MODIFIED Requirements

### Requirement: Face rendering
The pal SHALL draw the current expression on the 200x200 black-and-white
e-paper panel: head, ears, spectacles and nose in black, with the
face-specific eyes and mouth, and Happy's open mouth as a black disc
with a white cut-out.

#### Scenario: Happy face
- **WHEN** the Happy face is drawn
- **THEN** the panel shows arc eyes and an open smile

#### Scenario: Other faces have no red
- **WHEN** any face is drawn
- **THEN** the picture contains only black and white pixels

### Requirement: Refresh pacing
The panel SHALL not refresh: closer than 2 s after the previous refresh,
or when neither the face nor the ladder band differs from what is
displayed. A face change lands about 400 ms after the last encoder
detent, so a continuous spin costs one refresh; a band change lands
within its sample period, a link loss at once, both behind the same gap.

#### Scenario: Unchanged face skipped
- **GIVEN** the panel shows Calm with the current band
- **WHEN** the face is set to Calm again
- **THEN** no refresh happens

#### Scenario: Spin costs one update
- **GIVEN** the last refresh is more than 2 s ago
- **WHEN** the knob is turned five detents in one second
- **THEN** exactly one refresh shows the final face

#### Scenario: Within the gap
- **GIVEN** a refresh completed less than 2 s ago
- **WHEN** the face or the band changes
- **THEN** the refresh is delayed until the 2 s gap has passed

#### Scenario: Nothing to show
- **GIVEN** the panel shows the current face and band
- **WHEN** an RSSI sample confirms the same band
- **THEN** no refresh happens

## REMOVED Requirements

### Requirement: Full refresh only
**Reason**: disproved on the real unit — the fast-partial waveform
updates the ladder window in 374 ms with no screen-wide flash
(`tools/partial_spike`, spike evidence).
**Migration**: replaced by "Fast ladder updates" and "Periodic full
clears" above.
