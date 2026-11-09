#include <unity.h>

#include "rotary_encoder.h"

void setUp() {}
void tearDown() {}

// One clockwise detent: 00 -> 01 -> 11 -> 10 -> 00.
static int cwStep(RotaryEncoder& encoder) {
  return encoder.update(true, false) + encoder.update(true, true) +
         encoder.update(false, true) + encoder.update(false, false);
}

static int ccwStep(RotaryEncoder& encoder) {
  return encoder.update(false, true) + encoder.update(true, true) +
         encoder.update(true, false) + encoder.update(false, false);
}

static void test_clockwise_emits_one_detent() {
  RotaryEncoder encoder;
  TEST_ASSERT_EQUAL_INT(0, encoder.update(false, false));
  TEST_ASSERT_EQUAL_INT(0, encoder.update(true, false));
  TEST_ASSERT_EQUAL_INT(0, encoder.update(true, true));
  TEST_ASSERT_EQUAL_INT(0, encoder.update(false, true));
  TEST_ASSERT_EQUAL_INT(1, encoder.update(false, false));
}

static void test_counter_clockwise_emits_one_detent() {
  RotaryEncoder encoder;
  TEST_ASSERT_EQUAL_INT(0, encoder.update(false, false));
  TEST_ASSERT_EQUAL_INT(0, encoder.update(false, true));
  TEST_ASSERT_EQUAL_INT(0, encoder.update(true, true));
  TEST_ASSERT_EQUAL_INT(0, encoder.update(true, false));
  TEST_ASSERT_EQUAL_INT(-1, encoder.update(false, false));
}

static void test_each_detent_emits_one_event() {
  RotaryEncoder encoder;
  TEST_ASSERT_EQUAL_INT(1, cwStep(encoder));
  TEST_ASSERT_EQUAL_INT(1, cwStep(encoder));
  TEST_ASSERT_EQUAL_INT(-1, ccwStep(encoder));
  TEST_ASSERT_EQUAL_INT(-1, ccwStep(encoder));
}

static void test_bounce_never_emits_a_detent() {
  RotaryEncoder encoder;
  for (int i = 0; i < 50; ++i) {
    TEST_ASSERT_EQUAL_INT(0, encoder.update(true, false));
    TEST_ASSERT_EQUAL_INT(0, encoder.update(false, false));
  }
}

static void test_half_turn_then_back_emits_nothing() {
  RotaryEncoder encoder;
  TEST_ASSERT_EQUAL_INT(0, encoder.update(false, true));
  TEST_ASSERT_EQUAL_INT(0, encoder.update(true, true));
  TEST_ASSERT_EQUAL_INT(0, encoder.update(false, true));
  TEST_ASSERT_EQUAL_INT(0, encoder.update(false, false));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_clockwise_emits_one_detent);
  RUN_TEST(test_counter_clockwise_emits_one_detent);
  RUN_TEST(test_each_detent_emits_one_event);
  RUN_TEST(test_bounce_never_emits_a_detent);
  RUN_TEST(test_half_turn_then_back_emits_nothing);
  return UNITY_END();
}
