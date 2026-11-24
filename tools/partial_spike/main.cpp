#include <Arduino.h>
#include <GxEPD2_BW.h>

#include "display.h"

// Spike: does the SSD1681 fast-partial waveform actually run on this unit?
// Toggles the planned signal-ladder window between 4 and 0 bars and times
// every update; a full refresh every 200 partials is the ghost control.
// Judge ghosting with eyes at count 200 and right after the full clear.

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
    GxEPD2_154_D67(kPanelCsPin, kPanelDcPin, kPanelRstPin, kPanelBusyPin));

namespace {

// The planned ladder corner; rotation 0 needs x and w as multiples of 8.
constexpr int16_t kLadderX = 160;
constexpr int16_t kLadderY = 4;
constexpr int16_t kLadderW = 40;
constexpr int16_t kLadderH = 24;
constexpr uint16_t kPartialsPerFull = 200;
constexpr uint32_t kToggleMs = 2000;

void drawLadder(uint8_t filled) {
  display.fillRect(kLadderX, kLadderY, kLadderW, kLadderH, GxEPD_WHITE);
  for (uint8_t b = 0; b < 4; ++b) {
    if (b >= filled) continue;
    const int16_t h = 6 * (b + 1);
    display.fillRect(kLadderX + 1 + b * 10, kLadderY + kLadderH - h, 8, h - 1,
                     GxEPD_BLACK);
  }
}

// Static high-contrast content for ghosts to show up against: a solid
// block and a dithered band, like the bear's fur marking.
void drawCard(uint8_t filled) {
  display.fillScreen(GxEPD_WHITE);
  display.drawRect(8, 40, 184, 152, GxEPD_BLACK);
  display.fillRect(60, 80, 80, 60, GxEPD_BLACK);
  for (int16_t y = 150; y < 186; y += 2) {
    for (int16_t x = 12; x < 188; x += 2) {
      display.drawPixel(x, y, GxEPD_BLACK);
    }
  }
  drawLadder(filled);
}

uint16_t partialCount = 0;

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0);
  while (!Serial && millis() < 2000) delay(10);
  display.init(0);
  display.setRotation(0);
  drawCard(4);
  const uint32_t t0 = micros();
  display.display(false);
  Serial.printf("spike: full refresh in %lu ms; toggling every 2 s\n",
                (micros() - t0) / 1000);
}

void loop() {
  static uint32_t lastToggleMs = 0;
  if (millis() - lastToggleMs < kToggleMs) return;
  lastToggleMs = millis();

  const uint8_t filled = (partialCount++ % 2) ? 0 : 4;
  drawLadder(filled);
  const uint32_t t0 = micros();
  display.displayWindow(kLadderX, kLadderY, kLadderW, kLadderH);
  Serial.printf("spike: partial %u -> %u bars in %lu ms\n", partialCount,
                filled, (micros() - t0) / 1000);

  if (partialCount % kPartialsPerFull == 0) {
    Serial.println("spike: judge ghosts in block and dither NOW, then watch");
    drawCard(filled);
    const uint32_t t1 = micros();
    display.display(false);
    Serial.printf("spike: full clear in %lu ms; ghosts should be gone\n",
                  (micros() - t1) / 1000);
  }
}
