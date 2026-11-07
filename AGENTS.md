# AGENTS.md

## Project

FrontiMote — an open-source desk mate shaped like a Venezuelan spectacled bear.
Firmware for the Seeed Studio XIAO ESP32S3 (ESP32-S3 @ 240 MHz, 8 MB flash,
320 KB RAM, 8 MB PSRAM). PlatformIO + Arduino framework, one board, no extra
hardware abstraction layers.

## Commands

```bash
pio run              # build
pio run -t upload    # flash over USB (460800 baud)
pio device monitor   # 115200, native USB CDC
pio test -e native   # host tests (Unity); never runs on the board
scripts/test-hooks.sh  # self-check for .githooks/commit-msg
```

Git hooks live in `.githooks/` (format check, commit message, build) and are
enabled by `pio run`, which sets `core.hooksPath`.

If upload fails: hold BOOT, tap RESET, release BOOT, upload again.

## Layout

- `src/main.cpp` — Arduino entry point: blink (`GPIO21`, active LOW),
  heartbeat, serial console (`on|off|status|help`)
- `include/line_reader.h` — pure C++ line parser, host-testable
- `test/test_line_reader/` — Unity tests for `include/` logic

## Coding rules

- Keep `loop()` non-blocking: no `delay()` outside `setup()`, no busy waits.
- Put logic that can run off-device in `include/` and cover it with a Unity test.
- Constants are `kFooBar`; file-local state lives in an anonymous namespace.
- 2-space indent, `camelCase` functions.
- Prefer readable code over comments. A comment documents the code it sits on,
  never plans, skills, MCP servers or process; keep it one line, no restating
  what the code already says.
- Use `LED_BUILTIN`; the LED is active LOW.
- Don't add dependencies, boards or config without asking.

## Working style — ponytail (level: full)

Smallest complete change that fully solves the task.

- No options, abstractions, wrappers or "for later" code nobody asked for.
- Reuse what is already in the repo; prefer deletion over addition.
- Finish the whole change: callers, tests, fixtures it breaks.
- New non-trivial logic gets one small test or assert-based self-check.
- A known limit gets a `shortcut: <limit>, <when to upgrade>` comment.

Never cut: validation, error handling, security, accessibility, real-hardware
calibration.

## Quality gates

- **Mandatory before finishing a non-trivial change:** `/ponytail-review`
  (logic, safety, load, tests, speed, over-engineering of this change).
- **On request, after a milestone, or before a release:** `/ponytail-audit`
  (whole repo: bugs, security, risky untested code, bloat).
- **Before a release:** `/ponytail-debt` (list deferred `shortcut:` items).
- Switch strictness with `/ponytail lite|full|ultra`.

## Docs

For ESP32 / ESP-IDF / Espressif SDK questions, use the Espressif docs MCP
server (`search_espressif_sources`) before answering from memory. Prefer
project files first; the MCP only covers current public Espressif docs.
