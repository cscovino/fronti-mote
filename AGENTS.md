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

Git hooks live in `.githooks/` (format, commit message, build) and are enabled
by `pio run`, which sets `core.hooksPath`. Commit headers are
`<emoji> type(scope): subject`, at most 72 characters; the emoji and type
tables live in `.githooks/commit-msg`, examples in `scripts/test-hooks.sh`.

If upload fails: hold BOOT, tap RESET, release BOOT, upload again.

## Layout

See README.md §Project layout for the file-by-file map.

## Planning

`openspec/specs/` holds the behavioral specs, `openspec/changes/` the
in-flight proposals. Work on them with the `openspec-*` skills, loaded by name,
not as slash commands.

## Hard rules

- Don't add dependencies, boards or config without asking.
- Keep history readable: fold intermediate steps (setup, verify, fixup) into
  as few self-contained commits as the topic allows, each revertible and
  understandable from its header alone.
- `include/secrets.h` is gitignored; Wi-Fi and MQTT credentials compile into
  flash as plain text.
- Prefer readable code over comments. A comment documents the code it sits on,
  never plans, skills, MCP servers or process; keep it one line, no restating
  what the code already says.
- For ESP32 / ESP-IDF / Espressif SDK questions, prefer project files first and
  use the Espressif docs MCP (`search_espressif_sources`) before answering from
  memory.

## Area rules

`src/AGENTS.md` and `include/AGENTS.md` carry the rules for those directories;
agents load them automatically when working there. Tools that do not discover
nested files: read both before editing in them.
