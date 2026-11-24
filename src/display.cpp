#include "display.h"

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <iterator>

#include "polygon.h"
#include "refresh_gate.h"

namespace {

// The bear's panel is a 1.54" black/white e-paper (GDEH0154D67-class,
// 200x200, SSD1681). A face changed needs the full waveform and flashes the
// screen; the ladder box alone updates with the fast partial waveform the
// spike (tools/partial_spike) proved usable. refresh_gate.h caps how often
// either happens.
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
    GxEPD2_154_D67(kPanelCsPin, kPanelDcPin, kPanelRstPin, kPanelBusyPin));

struct PanelRequest {
  Expression face;
  uint8_t bands;
};

QueueHandle_t panelQueue = nullptr;

// Top-right corner, the only region every face picture leaves free.
// Rotation 0 window updates want x and w as multiples of 8.
constexpr int16_t kLadderX = 160;
constexpr int16_t kLadderY = 4;
constexpr int16_t kLadderW = 40;
constexpr int16_t kLadderH = 24;
constexpr uint16_t kPartialsPerFull = 500;

// The eyes and the dark sockets around them must share one x.
constexpr int16_t kEyeLeft = 63;
constexpr int16_t kEyeRight = 136;

void line2(Adafruit_GFX& g, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
           uint16_t color) {
  g.drawLine(x0, y0, x1, y1, color);
  g.drawLine(x0, y0 + 1, x1, y1 + 1, color);
}

void fillPolygon(Adafruit_GFX& g, const Point* pts, uint8_t count,
                 uint16_t color) {
  forEachPolygonSpan(pts, count, [&](int16_t y, int16_t x0, int16_t x1) {
    g.drawFastHLine(x0, y, x1 - x0 + 1, color);
  });
}

// Half-covered pixels: the panel shows them as grey, which reads as pale fur.
void fillFur(Adafruit_GFX& g, const Point* pts, uint8_t count, uint16_t color) {
  forEachPolygonSpan(pts, count, [&](int16_t y, int16_t x0, int16_t x1) {
    for (int16_t x = x0; x <= x1; ++x) {
      if (((x + y) & 1) == 0) g.drawPixel(x, y, color);
    }
  });
}

// Right half of a shape, from the hand-drawn left half, keeping the face
// symmetric; the winding flips but the even-odd fill does not care.
void mirror(const Point* left, uint8_t count, Point* right) {
  for (uint8_t i = 0; i < count; ++i) {
    right[i] = {int16_t(GxEPD2_154_D67::WIDTH - 1 - left[i].x), left[i].y};
  }
}

// A circle sampled at two radii and interleaved: loosePercent below 100
// breaks the outline up like fur, 100 gives a plain ellipse.
void furRing(Point* out, int16_t cx, int16_t cy, int16_t rx, int16_t ry,
             uint8_t loosePercent = 100) {
  Point taut[kMaxPolygonPoints];
  Point loose[kMaxPolygonPoints];
  ellipsePoints(cx, cy, rx, ry, taut, kMaxPolygonPoints);
  ellipsePoints(cx, cy, rx * loosePercent / 100, ry * loosePercent / 100, loose,
                kMaxPolygonPoints);
  for (uint8_t i = 0; i < kMaxPolygonPoints; ++i) {
    out[i] = (i & 1) ? loose[i] : taut[i];
  }
}

void drawEar(Adafruit_GFX& g) {
  Point ear[kMaxPolygonPoints];
  Point inner[kMaxPolygonPoints];
  for (const int16_t cx : {26, 174}) {
    furRing(ear, cx, 50, 21, 20);
    fillPolygon(g, ear, kMaxPolygonPoints, GxEPD_BLACK);
    furRing(inner, cx, 52, 11, 10, 88);
    fillFur(g, inner, kMaxPolygonPoints, GxEPD_WHITE);
  }
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

// The pale markings of a spectacled bear: spectacles, bridge, muzzle.
void drawMarkings(Adafruit_GFX& g) {
  Point ring[kMaxPolygonPoints];

  constexpr Point kPlate[] = {{34, 96}, {40, 68},  {58, 54},  {84, 62},
                              {96, 86}, {88, 110}, {58, 118}, {40, 110}};
  Point plate[kMaxPolygonPoints];
  mirror(kPlate, std::size(kPlate), plate);
  fillPolygon(g, kPlate, std::size(kPlate), GxEPD_WHITE);
  fillPolygon(g, plate, std::size(kPlate), GxEPD_WHITE);

  constexpr Point kBridge[] = {{84, 82}, {116, 82}, {112, 94}, {88, 94}};
  fillPolygon(g, kBridge, std::size(kBridge), GxEPD_WHITE);

  // Eye sockets, so the drawn eyes sit in dark skin again.
  for (const int16_t cx : {kEyeLeft, kEyeRight}) {
    furRing(ring, cx, 89, 23, 22);
    fillPolygon(g, ring, kMaxPolygonPoints, GxEPD_BLACK);
  }

  furRing(ring, 100, 152, 50, 36);
  fillFur(g, ring, kMaxPolygonPoints, GxEPD_WHITE);
  furRing(ring, 100, 154, 36, 24);
  fillPolygon(g, ring, kMaxPolygonPoints, GxEPD_WHITE);

  // Broad nose pad, flat on top and rounded underneath.
  constexpr Point kNose[] = {{78, 118},  {122, 118}, {126, 127}, {120, 136},
                             {100, 141}, {80, 136},  {74, 127}};
  fillPolygon(g, kNose, std::size(kNose), GxEPD_BLACK);
}

// Phone-style ladder: band b fills the bars 0..b-1, band 0 the bare box.
void drawLadder(Adafruit_GFX& g, uint8_t bands) {
  g.fillRect(kLadderX, kLadderY, kLadderW, kLadderH, GxEPD_WHITE);
  for (uint8_t b = 0; b < 4; ++b) {
    if (b >= bands) continue;
    const int16_t h = 6 * (b + 1);
    g.fillRect(kLadderX + 1 + b * 10, kLadderY + kLadderH - h, 8, h - 1,
               GxEPD_BLACK);
  }
}

void drawFace(Adafruit_GFX& g, Expression face, uint8_t bands) {
  g.fillScreen(GxEPD_WHITE);

  // Crown narrow, cheeks wide, chin blunt: a bear skull, not a circle.
  constexpr Point kHead[] = {{80, 30},   {120, 30},  {152, 40},  {176, 66},
                             {186, 100}, {184, 136}, {170, 166}, {142, 186},
                             {112, 194}, {88, 194},  {58, 186},  {30, 166},
                             {16, 136},  {14, 100},  {24, 66},   {48, 40}};
  drawEar(g);
  fillPolygon(g, kHead, std::size(kHead), GxEPD_BLACK);

  drawMarkings(g);

  drawEye(g, kEyeLeft, face);
  drawEye(g, kEyeRight, face);
  drawMouth(g, face);
  drawLadder(g, bands);
}

void displayTask(void*) {
  // A non-zero baud makes GxEPD2 call Serial.begin() and log busy waits.
  display.init(0);
  display.setRotation(0);
  PanelRequest shown{Expression::Calm, 0};
  bool haveShown = false;
  uint16_t partialsSinceFull = 0;
  uint32_t lastRefreshMs = 0;
  for (;;) {
    PanelRequest req;
    xQueueReceive(panelQueue, &req, portMAX_DELAY);
    PanelRequest newer;
    while (xQueueReceive(panelQueue, &newer, 0) == pdTRUE) req = newer;

    const uint32_t waitMs = refreshWaitMs(haveShown, millis(), lastRefreshMs);
    if (waitMs) {
      vTaskDelay(pdMS_TO_TICKS(waitMs));
      while (xQueueReceive(panelQueue, &newer, 0) == pdTRUE) req = newer;
    }
    if (haveShown && req.face == shown.face && req.bands == shown.bands) {
      continue;
    }

    // A face changed everywhere, so the flashing full waveform; a ladder
    // alone updates its box silently until the spike-proven ghost budget
    // of 500 partials is spent.
    const bool partial = haveShown && req.face == shown.face &&
                         partialsSinceFull < kPartialsPerFull;
    if (partial) {
      drawLadder(display, req.bands);
      display.displayWindow(kLadderX, kLadderY, kLadderW, kLadderH);
      ++partialsSinceFull;
    } else {
      drawFace(display, req.face, req.bands);
      display.display(false);
      partialsSinceFull = 0;
    }
    shown = req;
    haveShown = true;
    lastRefreshMs = millis();
  }
}

}  // namespace

void displayBegin() {
  panelQueue = xQueueCreate(1, sizeof(PanelRequest));
  if (!panelQueue) {
    Serial.println("display: queue create failed");
    return;
  }
  if (xTaskCreatePinnedToCore(displayTask, "display", 4096, nullptr, 1, nullptr,
                              0) != pdPASS) {
    Serial.println("display: task start failed");
  }
}

void displayShow(Expression face, uint8_t bands) {
  if (panelQueue) {
    const PanelRequest req{face, bands};
    xQueueOverwrite(panelQueue, &req);
  }
}
