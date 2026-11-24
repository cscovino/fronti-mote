# Proposal

## Why

The firmware already has six working behaviors (faces, encoder, buzzer, e-paper
refresh, Wi-Fi, MQTT) documented only in prose in the README. Future changes
need agreed requirements to diff against, so the current behavior becomes the
baseline specs — a documentation-only backfill, no code changes.

## What Changes

- Add capability specs that describe the observable behavior of the firmware as
  it runs today, derived from the code and the host tests:
  - `expressions` — the four faces, their order, wrapping, names and notes.
  - `encoder-input` — quadrature decoding into ±1 detents with bounce rejection.
  - `buzzer` — tone generation, per-detent note, tune playback, link tunes.
  - `epaper-display` — full-refresh policy: settle, 2 s gap, unchanged-face skip,
    background refresh task.
  - `wifi-link` — join, retry, link debounce, LED patterns, async scan.
  - `serial-console` — command set, echo, line limits, heartbeat, boot banner.
  - `mqtt-commands` — command JSON, state publish, last will, broker retry.
- No implementation task changes code; applying this change means validating the
  deltas and archiving them into `openspec/specs/`.

## Capabilities

### New Capabilities

- `expressions`: face set, order, wrap-around cycling, name/note lookup.
- `encoder-input`: rotary A/B quadrature decoding into detent steps.
- `buzzer`: passive-buzzer tones and tunes, including link up/down tunes.
- `epaper-display`: when and how the 1.54" three-color panel is refreshed.
- `wifi-link`: station-mode connect/retry behavior and its LED + tune signals.
- `serial-console`: interactive serial commands and heartbeat logging.
- `mqtt-commands`: MQTT command parsing, state publishing and last will.

### Modified Capabilities

(none — no existing specs)

## Impact

- Only `openspec/` artifacts; `src/`, `include/` and `test/` are untouched.
- Specs become the reference contract for future change proposals; README stays
  the human-facing guide.
- `wifi-link` and `mqtt-commands` describe the Wi-Fi branch, whose code is not on
  `main`; their deltas are held back from this baseline and land with that
  feature branch.
