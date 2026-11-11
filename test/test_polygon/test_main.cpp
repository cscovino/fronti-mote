#include <unity.h>

#include <cmath>
#include <vector>

#include "polygon.h"

void setUp() {}
void tearDown() {}

namespace {

// Records the spans a rasterizer produced, as (y, x0, x1) triples. The span
// walker takes its fill by value, so it writes through a reference.
struct Recorder {
  std::vector<int16_t>& out;
  void operator()(int16_t y, int16_t x0, int16_t x1) {
    out.insert(out.end(), {y, x0, x1});
  }
};

std::vector<int16_t> spans(const Point* pts, uint8_t count) {
  std::vector<int16_t> log;
  forEachPolygonSpan(pts, count, Recorder{log});
  return log;
}

}  // namespace

static void test_rectangle_covers_every_row() {
  const Point square[] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}};
  const std::vector<int16_t> got = spans(square, 4);
  TEST_ASSERT_EQUAL_UINT16(10 * 3, got.size());
  for (int16_t y = 0; y < 10; ++y) {
    TEST_ASSERT_EQUAL_INT16(y, got[y * 3]);
    TEST_ASSERT_EQUAL_INT16(0, got[y * 3 + 1]);
    TEST_ASSERT_EQUAL_INT16(9, got[y * 3 + 2]);
  }
}

static void test_triangle_shrinks_by_one_pixel_per_row() {
  const Point tri[] = {{0, 0}, {10, 0}, {0, 10}};
  const std::vector<int16_t> got = spans(tri, 3);
  TEST_ASSERT_EQUAL_UINT16(10 * 3, got.size());
  TEST_ASSERT_EQUAL_INT16(9, got[2]);
  TEST_ASSERT_EQUAL_INT16(0, got[9 * 3 + 2]);
}

static void test_concave_shape_has_two_spans_per_row() {
  // L shape: the full width over the top arm, then only the stem.
  const Point leg[] = {{0, 0}, {10, 0}, {10, 4}, {4, 4}, {4, 10}, {0, 10}};
  const std::vector<int16_t> got = spans(leg, 6);
  TEST_ASSERT_EQUAL_UINT16(10 * 3, got.size());
  TEST_ASSERT_EQUAL_INT16(9, got[2]);
  TEST_ASSERT_EQUAL_INT16(3, got[4 * 3 + 2]);
}

static void test_ellipse_points_stay_on_the_radii() {
  Point ring[kMaxPolygonPoints];
  ellipsePoints(100, 100, 40, 20, ring, kMaxPolygonPoints);
  for (uint8_t i = 0; i < kMaxPolygonPoints; ++i) {
    const int16_t dx = std::abs(ring[i].x - 100);
    const int16_t dy = std::abs(ring[i].y - 100);
    TEST_ASSERT_TRUE_MESSAGE(dx <= 40 && dy <= 20, "point off ellipse");
  }
  const std::vector<int16_t> got = spans(ring, kMaxPolygonPoints);
  TEST_ASSERT_TRUE(got.size() >= 40 * 3);
}

static void test_point_budget_is_clamped_not_overflowed() {
  Point ring[kMaxPolygonPoints];
  ellipsePoints(50, 50, 10, 10, ring, kMaxPolygonPoints + 40);
  const std::vector<int16_t> got = spans(ring, kMaxPolygonPoints);
  TEST_ASSERT_EQUAL_UINT16(20 * 3, got.size());
  for (size_t i = 0; i < got.size(); i += 3) {
    TEST_ASSERT_TRUE(got[i + 1] >= 40 && got[i + 2] <= 60);
  }
}

static void test_a_sharp_tip_emits_no_empty_span() {
  // The apex at (50,0) crosses the first scanline inside a single pixel; an
  // empty span there would make drawFastHLine draw one stray pixel.
  const Point tip[] = {{0, 100}, {50, 0}, {100, 100}};
  const std::vector<int16_t> got = spans(tip, 3);
  TEST_ASSERT_EQUAL_UINT16(99 * 3, got.size());
  for (size_t i = 0; i < got.size(); i += 3) {
    TEST_ASSERT_TRUE(got[i] >= 1);
    TEST_ASSERT_TRUE(got[i + 1] <= got[i + 2]);
  }
}

static void test_degenerate_shapes_draw_nothing() {
  const Point segment[] = {{0, 0}, {10, 10}};
  TEST_ASSERT_EQUAL_UINT16(0, spans(segment, 2).size());
  // Collinear points: every span comes out empty.
  const Point flat[] = {{0, 0}, {20, 0}, {10, 0}};
  TEST_ASSERT_EQUAL_UINT16(0, spans(flat, 3).size());
  Point ring[kMaxPolygonPoints] = {{7, 7}, {7, 7}, {7, 7}, {7, 7}};
  ellipsePoints(0, 0, 0, 0, ring, 0);
  TEST_ASSERT_EQUAL_INT16(7, ring[0].x);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_rectangle_covers_every_row);
  RUN_TEST(test_triangle_shrinks_by_one_pixel_per_row);
  RUN_TEST(test_concave_shape_has_two_spans_per_row);
  RUN_TEST(test_ellipse_points_stay_on_the_radii);
  RUN_TEST(test_degenerate_shapes_draw_nothing);
  RUN_TEST(test_a_sharp_tip_emits_no_empty_span);
  RUN_TEST(test_point_budget_is_clamped_not_overflowed);
  return UNITY_END();
}
