#include <unity.h>

#include "refresh_gate.h"

void setUp() {}
void tearDown() {}

static void test_the_first_face_refreshes_at_once() {
  TEST_ASSERT_EQUAL_UINT32(0, refreshWaitMs(false, 120, 0));
}

static void test_a_wait_is_counted_down() {
  TEST_ASSERT_EQUAL_UINT32(1500, refreshWaitMs(true, 500, 0));
}

static void test_the_gap_leaves_nothing_to_wait() {
  TEST_ASSERT_EQUAL_UINT32(0, refreshWaitMs(true, kMinRefreshGapMs, 0));
  TEST_ASSERT_EQUAL_UINT32(0, refreshWaitMs(true, kMinRefreshGapMs + 9000, 0));
}

static void test_the_gap_survives_a_millis_rollover() {
  TEST_ASSERT_EQUAL_UINT32(1892, refreshWaitMs(true, 100, 0xFFFFFFF8u));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_the_first_face_refreshes_at_once);
  RUN_TEST(test_a_wait_is_counted_down);
  RUN_TEST(test_the_gap_leaves_nothing_to_wait);
  RUN_TEST(test_the_gap_survives_a_millis_rollover);
  return UNITY_END();
}
