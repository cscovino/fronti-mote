#include "display.h"

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace {

constexpr uint8_t kCsPin = D0;    // GPIO1
constexpr uint8_t kDcPin = D1;    // GPIO2
constexpr uint8_t kRstPin = D2;   // GPIO3
constexpr uint8_t kBusyPin = D9;  // GPIO8, SPI MISO is unused

// Waveshare 1.54" V2 partial waveform from Espressif's esp_lcd_ssd1681 demo.
// 153 bytes go to 0x32; the last six are the 0x3F/0x03/0x04/0x2C registers.
const uint8_t kFastLut[159] = {
    0x80, 0x48, 0x40, 0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,  0x0,  0x0,  0x40,
    0x48, 0x80, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,  0x0,  0x80, 0x48,
    0x40, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,  0x40, 0x48, 0x80,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0a, 0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0,  0x0,  0x0,  0x0,
    0x0,  0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x0, 0x0, 0x0,  0x22, 0x17, 0x41,
    0x0,  0x32, 0x20,
};

// Replaces the OTP differential waveform with the vendor partial one, so face
// changes refresh in ~0.5 s instead of flashing the whole panel for ~2.6 s.
class WavesharePanel : public GxEPD2_154_D67 {
 public:
  using GxEPD2_154_D67::GxEPD2_154_D67;

  void refresh(bool partial_update_mode) override {
    if (!partial_update_mode && fastLutLoaded_) {
      // Reload the factory waveform our partial LUT replaced (esp_lcd_ssd1681
      // does the same when it switches the panel back on).
      _writeCommand(0x22);
      _writeData(0xB1);
      _writeCommand(0x3F);
      _writeData(0x07);
      _writeCommand(0x20);
      _waitWhileBusy("_loadOtpLut", 1000);
      _writeCommand(0x3C);
      _writeData(0x05);
      fastLutLoaded_ = false;
    }
    GxEPD2_154_D67::refresh(partial_update_mode);
  }

  void refresh(int16_t x, int16_t y, int16_t w, int16_t h) override {
    if (_initial_refresh) {
      GxEPD2_154_D67::refresh(x, y, w, h);
      return;
    }
    int16_t w1 = x < 0 ? w + x : w;
    int16_t h1 = y < 0 ? h + y : h;
    int16_t x1 = x < 0 ? 0 : x;
    int16_t y1 = y < 0 ? 0 : y;
    w1 = x1 + w1 < int16_t(WIDTH) ? w1 : int16_t(WIDTH) - x1;
    h1 = y1 + h1 < int16_t(HEIGHT) ? h1 : int16_t(HEIGHT) - y1;
    if (w1 <= 0 || h1 <= 0) return;
    w1 += x1 % 8;
    if (w1 % 8 > 0) w1 += 8 - w1 % 8;
    x1 -= x1 % 8;
    setRamArea(x1, y1, w1, h1);

    if (!fastLutLoaded_) {
      loadFastLut();
      fastLutLoaded_ = true;
    }
    _writeCommand(0x22);
    _writeData(0xC0);  // power on
    _writeCommand(0x20);
    _waitWhileBusy("_PowerOn", 1000);
    _writeCommand(0x22);
    _writeData(0xCF);  // display using the loaded LUT
    _writeCommand(0x20);
    _waitWhileBusy("_Update_Part", 1000);
    _power_is_on = false;  // 0xCF already disabled analog and the oscillator
  }

 private:
  // The LUT registers keep their bytes until the OTP reload or a reset.
  bool fastLutLoaded_ = false;

  void loadFastLut() {
    _writeCommand(0x32);
    for (uint8_t i = 0; i < 153; ++i) _writeData(kFastLut[i]);
    _writeCommand(0x3F);
    _writeData(kFastLut[153]);
    _writeCommand(0x03);
    _writeData(kFastLut[154]);
    _writeCommand(0x04);
    _writeData(kFastLut[155]);
    _writeData(kFastLut[156]);
    _writeData(kFastLut[157]);
    _writeCommand(0x2C);
    _writeData(kFastLut[158]);
    // Waveshare writes these option bytes right before the partial update.
    _writeCommand(0x37);
    const uint8_t kOption[10] = {0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x40, 0x00, 0x00, 0x00, 0x00};
    _writeData(kOption, sizeof(kOption));
    _writeCommand(0x3C);  // border waveform used by partial refresh
    _writeData(0x80);
    _waitWhileBusy("_loadFastLut", 1000);
  }

  // GxEPD2_154_D67::_setPartialRamArea is private; this is the same sequence.
  void setRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    _writeCommand(0x11);
    _writeData(0x03);
    _writeCommand(0x44);
    _writeData(x / 8);
    _writeData((x + w - 1) / 8);
    _writeCommand(0x45);
    _writeData(y % 256);
    _writeData(y / 256);
    _writeData((y + h - 1) % 256);
    _writeData((y + h - 1) / 256);
    _writeCommand(0x4E);
    _writeData(x / 8);
    _writeCommand(0x4F);
    _writeData(y % 256);
    _writeData(y / 256);
  }
};

GxEPD2_BW<WavesharePanel, 200> display(WavesharePanel(kCsPin, kDcPin, kRstPin,
                                                      kBusyPin));

QueueHandle_t faceQueue = nullptr;

constexpr uint8_t kFullRefreshEvery = 10;

// Only the eyes and mouth change between faces; the rest of the head is static.
constexpr int16_t kFaceX = 40;
constexpr int16_t kFaceY = 64;
constexpr int16_t kFaceW = 120;
constexpr int16_t kFaceH = 120;

void line2(Adafruit_GFX& g, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
           uint16_t color) {
  g.drawLine(x0, y0, x1, y1, color);
  g.drawLine(x0, y0 + 1, x1, y1 + 1, color);
}

void drawEye(Adafruit_GFX& g, int16_t x, Expression face) {
  constexpr int16_t kY = 88;
  switch (face) {
    case Expression::Sleepy:
      line2(g, x - 9, kY, x + 9, kY, GxEPD_WHITE);
      break;
    case Expression::Calm:
      g.fillCircle(x, kY, 8, GxEPD_WHITE);
      g.fillCircle(x, kY, 4, GxEPD_BLACK);
      break;
    case Expression::Happy:
      line2(g, x - 9, kY + 6, x, kY - 3, GxEPD_WHITE);
      line2(g, x, kY - 3, x + 9, kY + 6, GxEPD_WHITE);
      break;
    case Expression::Surprised:
      g.fillCircle(x, kY, 13, GxEPD_WHITE);
      g.fillCircle(x, kY, 7, GxEPD_BLACK);
      g.fillCircle(x, kY, 3, GxEPD_WHITE);
      break;
  }
}

void drawMouth(Adafruit_GFX& g, Expression face) {
  constexpr int16_t kCx = 100;
  constexpr int16_t kMy = 160;
  switch (face) {
    case Expression::Sleepy:
      line2(g, kCx - 12, kMy, kCx + 12, kMy, GxEPD_BLACK);
      break;
    case Expression::Calm:
      line2(g, kCx - 14, kMy - 6, kCx - 7, kMy + 1, GxEPD_BLACK);
      line2(g, kCx - 7, kMy + 1, kCx + 7, kMy + 1, GxEPD_BLACK);
      line2(g, kCx + 7, kMy + 1, kCx + 14, kMy - 6, GxEPD_BLACK);
      break;
    case Expression::Happy:
      g.fillCircle(kCx, kMy - 2, 15, GxEPD_BLACK);
      g.fillRect(kCx - 16, kMy - 17, 33, 15, GxEPD_WHITE);
      break;
    case Expression::Surprised:
      g.drawCircle(kCx, kMy, 7, GxEPD_BLACK);
      g.drawCircle(kCx, kMy, 6, GxEPD_BLACK);
      break;
  }
}

void drawFace(Adafruit_GFX& g, Expression face) {
  g.fillScreen(GxEPD_WHITE);
  g.fillCircle(38, 40, 30, GxEPD_BLACK);
  g.fillCircle(162, 40, 30, GxEPD_BLACK);
  g.fillCircle(100, 108, 84, GxEPD_BLACK);
  g.fillCircle(100, 150, 40, GxEPD_WHITE);  // muzzle

  for (int16_t x : {64, 136}) {  // spectacles
    g.fillCircle(x, 88, 26, GxEPD_WHITE);
    g.fillCircle(x, 88, 21, GxEPD_BLACK);
  }
  g.fillRect(88, 85, 24, 6, GxEPD_WHITE);

  drawEye(g, 64, face);
  drawEye(g, 136, face);

  g.fillCircle(100, 128, 9, GxEPD_BLACK);
  g.fillTriangle(93, 128, 107, 128, 100, 140, GxEPD_BLACK);
  drawMouth(g, face);
}

void displayTask(void*) {
  display.init(0);
  display.setRotation(0);
  uint8_t partials = 0;
  bool first = true;
  for (;;) {
    Expression face;
    xQueueReceive(faceQueue, &face, portMAX_DELAY);
    Expression newer;
    while (xQueueReceive(faceQueue, &newer, 0) == pdTRUE) face = newer;

    drawFace(display, face);
    if (first || partials >= kFullRefreshEvery) {
      display.display(false);
      first = false;
      partials = 0;
    } else {
      display.displayWindow(kFaceX, kFaceY, kFaceW, kFaceH);
      ++partials;
    }
  }
}

}  // namespace

void displayBegin() {
  faceQueue = xQueueCreate(1, sizeof(Expression));
  if (!faceQueue) {
    Serial.println("display: queue create failed");
    return;
  }
  if (xTaskCreatePinnedToCore(displayTask, "display", 4096, nullptr, 1, nullptr,
                              0) != pdPASS) {
    Serial.println("display: task start failed");
  }
}

void displayShow(Expression face) {
  if (faceQueue) {
    xQueueOverwrite(faceQueue, &face);
  }
}
