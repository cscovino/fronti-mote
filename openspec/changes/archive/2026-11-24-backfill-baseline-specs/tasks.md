# Tasks

## 1. Verify the deltas against the firmware

- [x] 1.1 Check `expressions` and `encoder-input` against `include/expressions.h`, `include/rotary_encoder.h`, `src/pal.cpp` and their tests; fix any wording that overstates behavior.
- [x] 1.2 Check `buzzer` and `epaper-display` against `src/pal.cpp`, `include/tunes.h`, `include/tune_player.h`, `include/refresh_gate.h`, `src/display.cpp`.
- [x] 1.3 Check `wifi-link` and `serial-console` against `src/wifi_link.cpp`, `include/link_state.h`, `src/console.cpp`, `include/line_reader.h`.
- [x] 1.4 Check `mqtt-commands` against `src/mqtt_link.cpp`, `include/mqtt_command.h` and `test/test_mqtt_command/`.
- [x] 1.5 Run `pio test -e native` and confirm every scenario that maps to a host test still passes.

## 2. Lock it in

- [ ] 2.1 Run `openspec validate backfill-baseline-specs` and fix format errors.
- [ ] 2.2 Commit the verified change folder.

## Workflow follow-up

- Archive the change so the deltas land in `openspec/specs/` (archive skill / `openspec archive`).
