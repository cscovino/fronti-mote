# FrontiMote

Your rubber duck upgraded. An open-source desk mate shaped like a Venezuelan spectacled bear 🐻.

## Board

| Board | MCU | Frequency | Flash | PSRAM | RAM |
| --- | --- | --- | --- | --- | --- |
| Seeed Studio XIAO ESP32S3 | ESP32-S3 | 240 MHz | 8 MB | 8 MB | 320 KB |

## Parts

Everything the firmware drives; all on the 3V3 rail. The XIAO ships
headerless: solder before wiring anything.

| Item | Notes |
| --- | --- |
| Seeed Studio XIAO ESP32S3 | the only board this firmware targets |
| 1.54" black/white e-paper, 200×200 | GDEH0154D67-class (SSD1681) — e.g. the Waveshare 1.54" V2 |
| Rotary encoder, EC11 class | A/B quadrature; the push-switch common is unused by firmware |
| Passive buzzer, 3–5 V | no internal oscillator; the ESP32 generates the tone |
| Q1: BC547B NPN transistor | low-side switch for the buzzer |
| D1: 1N4007 diode | flyback across the buzzer, cathode to 3V3 |
| R1: 1 kΩ resistor | in series with Q1's base |
| Male pin headers | the XIAO and bare EC11s ship without them |
| Breadboard, jumpers, USB-C data cable | |

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

The firmware gives you:

- a 1.54" e-paper bear face with four expressions,
- a rotary encoder that changes the expression and plays a note for each detent
  on a passive buzzer,
- a Wi-Fi link whose state the onboard LED (`GPIO21`, active LOW) shows and a
  short tune announces,
- a heartbeat every 2 seconds (`status` adds chip, flash, heap and Wi-Fi),
- a tiny echoing serial console: type `face`, `buzz`, `wifi`, `status`, or
  `help`.

## Wi-Fi

The board joins one network in station mode. Credentials live in
`include/secrets.h`, which is gitignored:

```bash
cp include/secrets.example.h include/secrets.h
$EDITOR include/secrets.h      # kWifiSsid, kWifiPassword (empty = open network)
```

Without that file the firmware still builds and runs; Wi-Fi stays off and the
console says so. The hostname is `frontimote-` plus the last six hex digits of
the MAC, the address comes from DHCP, and modem sleep is disabled so the link
stays responsive. The station reconnects by itself after a drop; a rejected
password is retried every 10 seconds. At boot the firmware scans the air once
(a few seconds inside `setup()`); the console `wifi` command starts the same
scan in the background, so the pal keeps reacting and the results print a few
seconds later.

The credentials are compiled into flash as plain text. Use a dedicated network:
anyone with the board and a USB cable can read the flash and recover them.

The LED is the link indicator:

| Link | LED |
| --- | --- |
| connected | solid on |
| connecting | fast blink (200 ms) |
| offline, wrong password, or no credentials | slow blink (1 s) |

A rising three-note tune plays when the link comes up, a falling two-note tune
when a link that was up goes down. A network that never connected stays silent,
so a wrong password does not chirp on every retry.

## MQTT

The pal subscribes to a broker so an agent or a script can make it react. The
settings live in the same gitignored `include/secrets.h`:

| Key | Meaning |
| --- | --- |
| `kMqttHost` | broker address; empty leaves MQTT off |
| `kMqttPort` | broker port, `1883` |
| `kMqttUser`, `kMqttPassword` | optional credentials |
| `kMqttPrefix` | topic root, `frontimote`; unique per unit on a shared broker |

The client connects once Wi-Fi is online and retries every 5 seconds; a failed
connect stalls the loop for at most the 3-second socket timeout. Plain TCP
only, no TLS. Commands are JSON on `<prefix>/cmd`:

```bash
mosquitto_pub -h broker -t frontimote/cmd -m '{"face":"happy"}'
mosquitto_pub -h broker -t frontimote/cmd -m '{"face":"next"}'
mosquitto_pub -h broker -t frontimote/cmd -m '{"beep":440}'
mosquitto_pub -h broker -t frontimote/cmd -m '{"tune":"up"}'
mosquitto_pub -h broker -t frontimote/cmd -m '{"status":true}'
```

- `face` — `sleepy`, `calm`, `happy`, `surprised` or `next`, case-insensitive.
- `beep` — one 200 ms note, 100 to 5000 Hz; anything outside is ignored.
- `tune` — `up`, `down`, `happy`, `sad`, `curious` or `sleepy`, case-insensitive.
  `up` and `down` are the link tunes; the other four are moods.
- `status` — publish the state now.

Malformed JSON and payloads with no recognized command are logged on the serial
console and otherwise ignored; payloads over the 512-byte receive buffer are
dropped without a log. The firmware publishes `<prefix>/state`
retained, on connect and on every face change, and registers
`{"online":false}` as its last will:

```json
{"online":true,"face":"calm","rssi":-52,"heap":123456,"uptime":1234}
```

Publish commands without `-r`: a retained command would replay on every boot.
Two units on one broker need two `kMqttPrefix` values; otherwise both obey every
command and overwrite the same state topic.

Any client that can publish to `<prefix>/cmd` can drive the pet, so give the
broker an ACL if the network is shared.

## Expressions

| Face | Eyes | Mouth | Note |
| --- | --- | --- | --- |
| Sleepy | closed lines | flat line | 262 Hz |
| Calm | dot pupils | small smile | 330 Hz |
| Happy | `^ ^` arcs | open smile, red tongue | 392 Hz |
| Surprised | big rings | small `o` | 523 Hz |

Turning the encoder wraps through the four faces in both directions. The panel
refreshes 400 ms after the last detent, and never twice inside 2 seconds or for
a face it already shows, so a spin costs one update.

## Drawing the face

Adafruit_GFX, which `GxEPD2_BW` derives from, has no polygon primitive, so
`include/polygon.h` walks the scanline spans of a simple polygon and
`src/display.cpp` fills them solid or stippled: the half-covered pixels read as
the pale fur of a spectacled bear's muzzle and ear insides. `mirror()` copies a
hand-drawn left half to the right, `furRing()` samples an ellipse at one or two
radii. The head, ears, spectacles and nose are point tables in `drawFace()`.

## E-paper display

The panel is a 1.54" black/white e-paper (200×200, SSD1681-class,
GDEH0154D67 or the equivalent Waveshare 1.54" V2): the firmware drives it
through `GxEPD2_154_D67`.

| E-paper | XIAO pin | GPIO |
| --- | --- | --- |
| VCC | 3V3 | — |
| GND | GND | — |
| DIN (SDI) | D10 | 9 |
| CLK (SCK) | D8 | 7 |
| CS | D0 | 1 |
| DC | D1 | 2 |
| RST | D2 | 3 |
| BUSY | D9 | 8 |

`D9` is also the SPI MISO pin, but the display only writes, so the BUSY input is
free to use it.

The panel can partial-refresh, but a face redraw fills the whole framebuffer,
so every change is a full refresh: the screen flashes black and white for a
few seconds. The only way to cut that is to refresh less:
`include/refresh_gate.h` holds the minimum 2 s gap and the skip of an
unchanged face. The refresh runs in a FreeRTOS task, so the encoder and buzzer
never wait for the panel.

Before shipping or returning the bear, blank the panel to white with the
one-shot firmware: `pio run -e clean -t upload` (source in
`tools/clean_display/`; it draws nothing and hibernates the panel).

## Encoder and buzzer

| Signal | XIAO pin | Notes |
| --- | --- | --- |
| Encoder A (CLK) | D4 (GPIO5) | `INPUT_PULLUP` in firmware |
| Encoder B (DT) | D5 (GPIO6) | `INPUT_PULLUP` in firmware |
| Encoder common | GND | |
| Encoder + | 3V3 | only if the module has its own pull-ups |
| Buzzer S (+) | 3V3 | passive buzzer |
| Buzzer (−) | BC547B collector | 1N4007 across the buzzer, cathode to 3V3 |
| BC547B base | D3 (GPIO4) via 1 kΩ | |
| BC547B emitter | GND | |

The buzzer is passive, so the ESP32 generates the tone with LEDC PWM. D4/D5 are
the I²C pins, so they are taken if you later add an I²C device.

### Breadboard sketch

```
XIAO ESP32S3          e-paper 1.54" black/white
3V3  ──────────────── VCC
GND  ──────────────── GND
D10  ──────────────── DIN
D8   ──────────────── CLK
D0   ──────────────── CS
D1   ──────────────── DC
D2   ──────────────── RST
D9   ──────────────── BUSY

3V3 ──┬─ encoder +          (only if the module has pull-ups)
      ├─ buzzer +
      └─ 1N4007 cathode
GND ──┬─ encoder C
      └─ Q1 emitter
D4  ──── encoder A (CLK)
D5  ──── encoder B (DT)
D3 ──[1k]── Q1 base          Q1 = BC547B, flat side up: C-B-E
Q1 collector ── buzzer − / 1N4007 anode
```

Only the 3V3 rail is used; nothing goes to 5V on either module.

If upload fails, put the board in bootloader mode: hold **BOOT**, tap **RESET**,
release **BOOT**, then upload again.

Git hooks (format check, commit message, firmware build) enable themselves on
the first `pio run`. If they ever go missing:
`git config core.hooksPath .githooks`. To turn them off again:
`git config --local core.hooksPath .git/hooks`.

## Project layout

- `src/main.cpp` — Arduino entry point: `setup()`/`loop()` wiring only
- `src/pal.cpp` — LED status, buzzer tunes, encoder, expression state and settle
  timer
- `src/wifi_link.cpp` — Wi-Fi join, retries, link status and tunes
- `src/mqtt_link.cpp` — MQTT client, command payloads and state
- `src/console.cpp` — serial console and heartbeat
- `src/display.cpp` — e-paper init, face drawing, background refresh task
- `include/expressions.h` — face order, names and notes, shared with the tests
- `include/display.h` — display start and face queue
- `include/pal.h` — pal hardware API
- `include/wifi_link.h` — Wi-Fi link API
- `include/mqtt_link.h` — MQTT link API
- `include/mqtt_command.h` — pure C++ JSON command parser, host-testable
- `include/tunes.h` — pure C++ tune table and name lookup, host-testable
- `include/console.h` — serial console API
- `include/link_state.h` — pure C++ link debounce and LED pattern, host-testable
- `include/tune_player.h` — pure C++ note sequencer shared with the tests
- `include/secrets.example.h` — Wi-Fi and MQTT settings template, copied to the
  gitignored `include/secrets.h`
- `include/line_reader.h` — pure C++ line parser shared with the host tests
- `include/polygon.h` — pure C++ polygon scanline fill shared with the tests
- `include/refresh_gate.h` — pure C++ e-paper refresh pacing, host-testable
- `include/rotary_encoder.h` — pure C++ quadrature decoder for the host tests
- `test/test_expressions/` — Unity tests for face wrapping and notes
- `test/test_line_reader/` — Unity tests for the parser
- `test/test_link_state/` — Unity tests for the debounce, retry and LED pattern
- `test/test_polygon/` — Unity tests for the scanline fill
- `test/test_refresh_gate/` — Unity tests for the refresh gap
- `test/test_rotary_encoder/` — Unity tests for the encoder decoder
- `test/test_mqtt_command/` — Unity tests for the MQTT payload parser
- `test/test_tunes/` — Unity tests for the tune table and names
- `test/test_tune_player/` — Unity tests for the note sequencer
- `tools/clean_display/` — one-shot panel blanker, flashed with
  `pio run -e clean -t upload`

## Tests

The face order, the line parser, the link state and retry, the polygon fill, the
refresh gate, the encoder decoder, the tune player, the tune names and the MQTT
payload parser run on the host:

```bash
pio test -e native
```

The board environment ignores tests (`test_ignore = *`), so nothing is flashed
during a test run.

The panel itself has no automated test. After a flash, check:

- boot shows the Calm face and the serial banner,
- turning the knob flashes the whole panel and lands on the new face,
- the face is black and white except the Happy face's red tongue; red that is
  missing or grey means the colour plane is not being driven.

## License

MIT. See [LICENSE](LICENSE).
