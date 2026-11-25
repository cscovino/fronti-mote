# Design

## Context

The panel's fast-partial waveform is proven on the real unit: windowed
updates of the ladder box took 374 ms with no screen-wide flash, and a
full refresh every 200 partials kept ghosts bounded (spike in
`tools/partial_spike/`). See proposal.md for motivation. The refresh task
in `src/display.cpp` owns the panel and its GFX buffer; `wifiTick()` owns
link state; the LED is a plain GPIO in `pal.cpp`.

## Goals / Non-Goals

**Goals:**
- Ladder visible on every full refresh, updated silently between them.
- All band math pure and host-testable, like the rest of `link_state.h`.
- One gate: every panel update (full or partial) obeys the 2 s gap and
  skips when nothing visible changed.

**Non-Goals:**
- No red/grey ladder variants, no per-packet RSSI polling, no changes to
  tunes, console, MQTT payloads, or the scan flow.

## Decisions

- **Queue payload becomes `{face, bars}`.** One `xQueueOverwrite` slot in
  the display task's `faceQueue`, extended to a small struct. The task
  compares against what it last showed: face differs -> full refresh
  (draws ladder anyway); only bars differ -> `displayWindow()` on the
  ladder box; neither -> skip. Alternative (separate icon queue) rejected:
  two sources racing one panel need one arbiter.
- **Band math in `link_state.h`:** `nextBand(cur, rssiDbm)` with
  thresholds -55/-65/-72/-80 dBm for bands 4/3/2/1, 0 = no link, and
  4 dB hysteresis (rising entry +4 dB above the falling threshold).
  `wifiTick()` samples RSSI every 15 s while Online; a band holds until
  the next sample confirms it (the 4 dB margin usually settles it in one).
  Alternative (continuous sampling) rejected: no perceptible gain at the
  cost of a busier loop.
- **On link up, sample RSSI immediately** so the up-refresh already shows
  the right ladder instead of guessing for 15 s. On link down, bars = 0
  feeds the same queue path: the drop shows without waiting.
- **Ladder geometry is the spike's:** box (x 160, y 4, w 40, h 24), bars
  at x 1 + b*10 (w 8) heights 6*(b+1) (last one inside the box). x and w
  are multiple-of-8 as `displayWindow` requires at rotation 0. One
  `drawLadder()` serves both the full-frame and the windowed path, so the
  framebuffer always matches the screen.
- **Forced full clear after K = 500 partials** — the spike ran 400 and the
  solid block and dither band still read clean, so the budget gets a
  margin. K is one constant to revisit after field use.
- **LED** in `palTick()`: `digitalWrite(status == Online ? LOW : HIGH)`,
  delete `ledOnForStatus()` and its blink tests. `LinkStatus`, debounce
  and retry logic are untouched.
- **2 s gap stays for every update** including partials: one rule, gentle
  on SPI busy-waits and panel wear.

## Risks / Trade-offs

- Ghost buildup between full clears on a jittery AP -> hysteresis keeps
  band changes rare; K=500 bounds worst case to hours of edge-of-band
  living.
- A band change racing a face change merges into one full refresh -> fine,
  queue overwrite already coalesces spins.
- `display.init()` first-update rules: the very first icon push must ride
  a full refresh; the task's existing `haveShown` flag covers it.
- The backfilled "three-color" spec wording will keep resurfacing until
  archive -> fixed in this change's delta.
