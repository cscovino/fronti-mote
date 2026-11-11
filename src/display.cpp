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

constexpr uint8_t kCsPin = D0;    // GPIO1
constexpr uint8_t kDcPin = D1;    // GPIO2
constexpr uint8_t kRstPin = D2;   // GPIO3
constexpr uint8_t kBusyPin = D9;  // GPIO8, SPI MISO is unused

// The bear's panel is a 1.54" black/white e-paper (GDEH0154D67-class,
// 200x200, SSD1681). A face redraw fills the whole framebuffer anyway, so
// every update is the flashing full-refresh waveform: the gap and the skip of
// unchanged faces in refresh_gate.h are the only ways to cut how often it
// flashes.
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
    GxEPD2_154_D67(kCsPin, kDcPin, kRstPin, kBusyPin));

QueueHandle_t faceQueue = nullptr;

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

void drawFace(Adafruit_GFX& g, Expression face) {
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
}

void displayTask(void*) {
  // A non-zero baud makes GxEPD2 call Serial.begin() and log busy waits.
  display.init(0);
  display.setRotation(0);
  Expression shown = Expression::Calm;
  bool haveShown = false;
  uint32_t lastRefreshMs = 0;
  for (;;) {
    Expression face;
    xQueueReceive(faceQueue, &face, portMAX_DELAY);
    Expression newer;
    while (xQueueReceive(faceQueue, &newer, 0) == pdTRUE) face = newer;

    const uint32_t waitMs = refreshWaitMs(haveShown, millis(), lastRefreshMs);
    if (waitMs) {
      vTaskDelay(pdMS_TO_TICKS(waitMs));
      while (xQueueReceive(faceQueue, &newer, 0) == pdTRUE) face = newer;
    }
    if (haveShown && face == shown) continue;

    drawFace(display, face);
    display.display(false);
    shown = face;
    haveShown = true;
    lastRefreshMs = millis();
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
