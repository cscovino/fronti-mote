# Proposal

## Why

The link indicator today is a blinking LED (fast/slow blink per state),
which is an anxious way to show "connected" and cannot show signal quality
at all. The bear's e-paper panel can carry a phone-style signal ladder, and
the spike (commit `1abf3ee`, `tools/partial_spike/`) proved the panel's
fast-partial waveform works on the real unit: ladder-window updates in
374 ms with no screen-wide flash, and a full refresh clears ghosts. That
replaces the spec's wrong "no usable partial waveform" claim with evidence
and unlocks an indicator that is both informative and quiet.

## What Changes

- **BREAKING** The LED stops blinking: solid on when online, off otherwise
  (connecting, offline and unconfigured are indistinguishable on the LED).
- New: a 4-bar RSSI ladder is drawn in the panel's top-right corner on every
  refresh and updated in place via windowed fast-partial refreshes.
- New: Wi-Fi link state gains an RSSI band (0-4) with hysteresis, sampled
  while online; band and link changes drive the ladder.
- Refresh pacing now treats an icon-state change as a refresh trigger
  (link change: immediate; band change: after a hold), still gated by the
  2 s minimum gap and never refreshing when face and icon are both
  unchanged.
- Corrects `epaper-display` wording to the actual black/white panel (the
  "three-color/red" text was a backfill error; the firmware driver is
  `GxEPD2_BW`).
- Console and MQTT command surface unchanged.

## Capabilities

### New Capabilities

(none)

### Modified Capabilities

- `wifi-link`: the "LED link indicator" requirement becomes a binary lamp;
  a new requirement adds RSSI band reporting with hysteresis; the purpose
  line changes from blink-based signalling to lamp + ladder.
- `epaper-display`: "Face rendering" corrected to black/white; "Full refresh
  only" replaced by full-for-faces / partial-for-ladder with a forced
  periodic full clear; "Refresh pacing" gains icon triggers; new
  "Signal ladder" requirement fixes geometry and offline look.

## Impact

- `include/link_state.h` — delete `ledOnForStatus`, add pure band function;
  `test/test_link_state/` grows cases.
- `src/display.cpp` + `include/display.h` — ladder draw, `displayWindow`
  partial path, icon-state plumbing through the refresh gate;
  `include/refresh_gate.h` unchanged in spirit, trigger set changes.
- `src/wifi_link.cpp` / `src/pal.cpp` — RSSI sampling every 15 s, band
  change and link change reporting; LED write becomes `status == Online`.
- `README.md` §display section — the partial-refresh paragraph becomes true
  firmware behaviour.
- Spike tool `tools/partial_spike/` stays as the evidence harness.
