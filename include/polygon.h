#pragma once

#include <cmath>
#include <cstdint>

struct Point {
  int16_t x;
  int16_t y;
};

constexpr uint8_t kMaxPolygonPoints = 16;

// Ellipse as a polygon, so spans can also fill stippled ellipses.
inline void ellipsePoints(int16_t cx, int16_t cy, int16_t rx, int16_t ry,
                          Point* out, uint8_t count) {
  if (count == 0) return;
  if (count > kMaxPolygonPoints) count = kMaxPolygonPoints;
  for (uint8_t i = 0; i < count; ++i) {
    const float angle = 6.2831853f * i / count;
    out[i] = {int16_t(std::lround(cx + rx * std::cos(angle))),
              int16_t(std::lround(cy + ry * std::sin(angle)))};
  }
}

// Calls fill(y, x0, x1) with an inclusive span for every covered scanline.
// Pixel centres are tested against the edges (even-odd), so concave shapes and
// shared edges have no seams. `pts` must hold `count` entries, 3 to
// kMaxPolygonPoints of them.
template <typename Span>
void forEachPolygonSpan(const Point* pts, uint8_t count, Span fill) {
  if (count < 3) return;
  if (count > kMaxPolygonPoints) count = kMaxPolygonPoints;
  int16_t top = pts[0].y;
  int16_t bottom = pts[0].y;
  for (uint8_t i = 1; i < count; ++i) {
    if (pts[i].y < top) top = pts[i].y;
    if (pts[i].y > bottom) bottom = pts[i].y;
  }
  for (int16_t y = top; y <= bottom; ++y) {
    const float row = y + 0.5f;
    float hits[kMaxPolygonPoints];
    uint8_t found = 0;
    for (uint8_t i = 0; i < count; ++i) {
      const Point& a = pts[i];
      const Point& b = pts[(i + 1) % count];
      if ((a.y < row) != (b.y < row)) {
        hits[found++] = a.x + (row - a.y) / (b.y - a.y) * (b.x - a.x);
      }
    }
    for (uint8_t i = 1; i < found; ++i) {
      const float key = hits[i];
      uint8_t j = i;
      while (j > 0 && hits[j - 1] > key) {
        hits[j] = hits[j - 1];
        --j;
      }
      hits[j] = key;
    }
    for (uint8_t i = 0; i + 1 < found; i += 2) {
      const int16_t x0 = int16_t(std::ceil(hits[i] - 0.5f));
      const int16_t x1 = int16_t(std::floor(hits[i + 1] - 0.5f));
      if (x0 > x1) continue;  // a tip narrower than one pixel covers nothing
      fill(y, x0, x1);
    }
  }
}
