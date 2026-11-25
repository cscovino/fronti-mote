#include <unity.h>

#include "tunes.h"

void setUp() {}
void tearDown() {}

static void test_every_name_resolves_to_its_tune() {
  for (uint8_t i = 0; i < kTuneCount; ++i) {
    Tune tune = Tune::Up;
    TEST_ASSERT_TRUE(tuneFromName(kTunes[i].name, &tune));
    TEST_ASSERT_EQUAL_INT(i, static_cast<int>(tune));
    TEST_ASSERT_EQUAL_STRING(kTunes[i].name, tuneInfo(tune).name);
  }
}

static void test_lookup_is_case_insensitive() {
  Tune tune = Tune::Down;
  TEST_ASSERT_TRUE(tuneFromName("HAPPY", &tune));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Tune::Happy), static_cast<int>(tune));
}

static void test_unknown_name_leaves_the_tune_alone() {
  Tune tune = Tune::Sleepy;
  TEST_ASSERT_FALSE(tuneFromName("grumpy", &tune));
  TEST_ASSERT_FALSE(tuneFromName("", &tune));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Tune::Sleepy), static_cast<int>(tune));
}

static void test_every_note_gets_an_audible_slot() {
  for (uint8_t i = 0; i < kTuneCount; ++i) {
    const TuneInfo& info = kTunes[i];
    TEST_ASSERT_GREATER_THAN_UINT8(0, info.len);
    for (uint8_t n = 0; n < info.len; ++n) {
      TEST_ASSERT_GREATER_THAN_UINT16(info.onMs, info.notes[n].slotMs);
    }
  }
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_every_name_resolves_to_its_tune);
  RUN_TEST(test_lookup_is_case_insensitive);
  RUN_TEST(test_unknown_name_leaves_the_tune_alone);
  RUN_TEST(test_every_note_gets_an_audible_slot);
  return UNITY_END();
}
