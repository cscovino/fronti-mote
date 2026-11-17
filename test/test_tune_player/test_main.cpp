#include <unity.h>

#include "tune_player.h"

void setUp() {}
void tearDown() {}

namespace {

constexpr uint16_t kOnMs = 80;
constexpr Note kTune[] = {{523, 160}, {659, 160}, {784, 160}};
constexpr uint8_t kLen = 3;

}  // namespace

static void test_the_first_note_sounds() {
  TEST_ASSERT_EQUAL_UINT16(523, tuneNoteHz(kTune, kLen, 0, kOnMs));
  TEST_ASSERT_EQUAL_UINT16(523, tuneNoteHz(kTune, kLen, kOnMs - 1, kOnMs));
}

static void test_each_note_leaves_a_gap() {
  TEST_ASSERT_EQUAL_UINT16(0, tuneNoteHz(kTune, kLen, kOnMs, kOnMs));
  TEST_ASSERT_EQUAL_UINT16(0, tuneNoteHz(kTune, kLen, 159, kOnMs));
}

static void test_the_next_note_starts_at_its_slot() {
  TEST_ASSERT_EQUAL_UINT16(659, tuneNoteHz(kTune, kLen, 160, kOnMs));
  TEST_ASSERT_EQUAL_UINT16(784, tuneNoteHz(kTune, kLen, 320, kOnMs));
}

static void test_the_tune_ends_in_silence() {
  TEST_ASSERT_EQUAL_UINT16(0, tuneNoteHz(kTune, kLen, 480, kOnMs));
  TEST_ASSERT_EQUAL_UINT16(0, tuneNoteHz(kTune, kLen, 60000, kOnMs));
}

static void test_the_length_sums_the_slots() {
  TEST_ASSERT_EQUAL_UINT32(480, tuneLengthMs(kTune, kLen));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_the_first_note_sounds);
  RUN_TEST(test_each_note_leaves_a_gap);
  RUN_TEST(test_the_next_note_starts_at_its_slot);
  RUN_TEST(test_the_tune_ends_in_silence);
  RUN_TEST(test_the_length_sums_the_slots);
  return UNITY_END();
}
