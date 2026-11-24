# Tasks

## 1. Band math (pure)

- [x] 1.1 Add `nextBand(cur, rssiDbm)` to `include/link_state.h` with
      thresholds -55/-65/-72/-80 dBm and 4 dB rising hysteresis; delete
      `ledOnForStatus()` and its constants' blink comment. Verify: new
      cases in `test/test_link_state` (steady, threshold hover, link-down
      to 0) and `pio test -e native` green.

## 2. Wi-Fi reports the band

- [x] 2.1 In `wifi_link.cpp`, sample RSSI every 15 s while Online (and
      once at link up), feed `nextBand`, and report band changes plus
      link-down-to-0 through a new `palLinkBand()` in `pal.h`/`pal.cpp`.
      Verify: with secrets configured, serial log shows one band line per
      change and none while stationary; boot shows the up-link band
      without a 15 s wait.

## 3. Ladder on the panel

- [x] 3.1 Add `drawLadder(bands)` in `display.cpp` at the spike geometry
      (box x 160-199, y 4-27; bars x 161+b*10, w 8, h 6*(b+1)-1), drawn by
      `drawFace` too; change the display queue payload to `{face, bands}`
      and `displayShow(face, bands)`. Verify: flashed build shows a 4-bar
      ladder once online and an empty box offline.
- [x] 3.2 In the display task, choose waveform by what changed: face ->
      full refresh; bands-only -> `displayWindow()` partial; force a full
      after 500 consecutive partials (reset by any full); extend the
      "nothing changed" skip to the struct. Verify: band change moves
      without screen-wide flash (~0.4 s); face change still flashes once;
      unchanged pushes refresh nothing (`pio test -e native` for the gate
      logic stays green, panel behaviour by eye).

## 4. Binary lamp

- [x] 4.1 In `palTick()`, drive the LED `Online ? LOW : HIGH` with no
      blink phase; delete dead blink state. Verify: holding a reboot
      (connecting) shows LED off, solid when online, off again when the
      router is unplugged; `pio run` green.

## 5. Docs and end-to-end

- [x] 5.1 Update the README display paragraph: ladder-only updates are
      partial, faces full, one full per 500 partials; remove the "the only
      way to cut that is to refresh less" framing. Verify: paragraph
      matches the shipped behaviour line by line.
- [ ] 5.2 Walk the delta-spec scenarios on the bear: steady signal costs
      zero refreshes; carry the bear across the room and the ladder steps
      without flashing; pull power from the AP and see LED off + empty box
      within the gap; plug in and see tune + solid LED + right band.
      Verify: every scenario in the two delta specs checked off here.

## Workflow follow-up

- At archive, also hand-edit the two Purpose lines to match the new
  reality (`wifi-link`: signal via lamp, ladder, tunes and scan;
  `epaper-display`: black/white panel, full-for-faces/partial-for-ladder)
  — Purpose changes live outside deltas.
- Archive the change with `openspec-archive-change` once 5.2 passes;
  deltas merge into `openspec/specs/`.
- Suggested commits: groups 1-2 `✨ feat(wifi)`, group 3
  `✨ feat(display)`, group 4 `🔄 refactor(display)` or folded into 3,
  group 5 `📝 docs`, archive `📝 docs(openspec)`.
