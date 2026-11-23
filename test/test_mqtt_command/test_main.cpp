#include <unity.h>

#include <cstring>

#include "mqtt_command.h"

void setUp() {}
void tearDown() {}

static PetCommand parse(const char* json) {
  return parsePetCommand(json, strlen(json));
}

static void test_face_by_name() {
  const PetCommand cmd = parse("{\"face\":\"happy\"}");
  TEST_ASSERT_TRUE(cmd.valid);
  TEST_ASSERT_TRUE(cmd.setFace);
  TEST_ASSERT_FALSE(cmd.nextFace);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Expression::Happy),
                        static_cast<int>(cmd.face));
}

static void test_face_is_case_insensitive() {
  const PetCommand cmd = parse("{\"face\":\"SURPRISED\"}");
  TEST_ASSERT_TRUE(cmd.setFace);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Expression::Surprised),
                        static_cast<int>(cmd.face));
}

static void test_unknown_face_asks_for_nothing() {
  const PetCommand cmd = parse("{\"face\":\"grumpy\"}");
  TEST_ASSERT_TRUE(cmd.valid);
  TEST_ASSERT_FALSE(cmd.setFace);
}

static void test_next_face() {
  TEST_ASSERT_TRUE(parse("{\"face\":\"next\"}").nextFace);
}

static void test_beep_in_range() {
  const PetCommand cmd = parse("{\"beep\":440}");
  TEST_ASSERT_TRUE(cmd.beep);
  TEST_ASSERT_EQUAL_UINT16(440, cmd.beepHz);
}

static void test_beep_out_of_range_is_dropped() {
  TEST_ASSERT_FALSE(parse("{\"beep\":10}").beep);
  TEST_ASSERT_FALSE(parse("{\"beep\":20000}").beep);
  TEST_ASSERT_FALSE(parse("{\"beep\":\"loud\"}").beep);
}

static void test_tune_by_name() {
  const PetCommand up = parse("{\"tune\":\"up\"}");
  TEST_ASSERT_TRUE(up.hasTune);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Tune::Up), static_cast<int>(up.tune));

  const PetCommand sad = parse("{\"tune\":\"SAD\"}");
  TEST_ASSERT_TRUE(sad.hasTune);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(Tune::Sad),
                        static_cast<int>(sad.tune));

  TEST_ASSERT_FALSE(parse("{\"tune\":\"sideways\"}").hasTune);
}

static void test_status_request() {
  TEST_ASSERT_TRUE(parse("{\"status\":true}").status);
  TEST_ASSERT_FALSE(parse("{\"status\":false}").status);
}

static void test_malformed_payload_is_marked_invalid() {
  TEST_ASSERT_FALSE(parse("not json").valid);
  TEST_ASSERT_FALSE(parse("").valid);
  TEST_ASSERT_FALSE(parse("{\"face\":").valid);
}

static void test_empty_object_asks_for_nothing() {
  const PetCommand cmd = parse("{}");
  TEST_ASSERT_TRUE(cmd.valid);
  TEST_ASSERT_FALSE(cmd.setFace || cmd.nextFace || cmd.beep || cmd.hasTune ||
                    cmd.status);
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_face_by_name);
  RUN_TEST(test_face_is_case_insensitive);
  RUN_TEST(test_unknown_face_asks_for_nothing);
  RUN_TEST(test_next_face);
  RUN_TEST(test_beep_in_range);
  RUN_TEST(test_beep_out_of_range_is_dropped);
  RUN_TEST(test_tune_by_name);
  RUN_TEST(test_status_request);
  RUN_TEST(test_malformed_payload_is_marked_invalid);
  RUN_TEST(test_empty_object_asks_for_nothing);
  return UNITY_END();
}
