# FrontiMote

Your rubber duck upgraded. An open-source desk mate shaped like a Venezuelan spectacled bear 🐻.

## Board

| Board | MCU | Frequency | Flash | RAM |
| --- | --- | --- | --- | --- |
| Seeed Studio XIAO ESP32S3 | ESP32-S3 | 240 MHz | 8 MB (8 MB PSRAM) | 320 KB |

## Requirements

- **Git**
- **PlatformIO Core** — the `pio` command on your `PATH`, from
  `pipx install platformio` or the
  [VS Code extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide).
  The ESP32 toolchain and Arduino framework download on the first build.
- **clang-format 18+** — the `pre-commit` hook checks the C++ you stage.
  Either name works (`clang-format` or `clang-format-18`):

  ```bash
  sudo apt install clang-format-18
  ```

## Getting started

```bash
pio run              # build
pio run -t upload    # flash over USB
pio device monitor   # 115200 baud, native USB CDC
```

`src/main.cpp` starts with:

- a non-blocking blink of the onboard LED (`GPIO21`, active LOW),
- a heartbeat + chip/heap report every 2 seconds,
- a tiny echoing serial console: type `on`, `off`, `status`, or `help`. The
  `on`/`off` commands stop the automatic blink and hold the requested state.

If upload fails, put the board in bootloader mode: hold **BOOT**, tap **RESET**,
release **BOOT**, then upload again.

Git hooks (format check, commit message, firmware build) enable themselves on
the first `pio run`. If they ever go missing:
`git config core.hooksPath .githooks`. To turn them off again:
`git config --local core.hooksPath .git/hooks`.

## Project layout

- `src/main.cpp` — Arduino entry point: blink, heartbeat, serial console
- `include/line_reader.h` — pure C++ line parser shared with the host tests
- `test/test_line_reader/` — Unity tests for the parser

## Tests

The serial line parser runs on the host:

```bash
pio test -e native
```

The board environment ignores tests (`test_ignore = *`), so nothing is flashed
during a test run.

## License

MIT. See [LICENSE](LICENSE).
