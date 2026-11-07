#include <unity.h>

#include <cstring>

#include "line_reader.h"

void setUp() {}
void tearDown() {}

static void expectEvent(LineEvent expected, LineEvent actual) {
  TEST_ASSERT_EQUAL_INT(static_cast<int>(expected), static_cast<int>(actual));
}

static void test_lf_ends_line() {
  LineReader reader;
  expectEvent(LineEvent::None, reader.feed('o'));
  expectEvent(LineEvent::None, reader.feed('n'));
  expectEvent(LineEvent::Line, reader.feed('\n'));
  TEST_ASSERT_EQUAL_STRING("on", reader.line());
}

static void test_cr_ends_line() {
  LineReader reader;
  reader.feed('o');
  reader.feed('f');
  reader.feed('f');
  expectEvent(LineEvent::Line, reader.feed('\r'));
  TEST_ASSERT_EQUAL_STRING("off", reader.line());
}

static void test_crlf_is_one_line() {
  LineReader reader;
  reader.feed('o');
  reader.feed('n');
  expectEvent(LineEvent::Line, reader.feed('\r'));
  expectEvent(LineEvent::None, reader.feed('\n'));
}

static void test_empty_lines_are_ignored() {
  LineReader reader;
  expectEvent(LineEvent::None, reader.feed('\n'));
  expectEvent(LineEvent::None, reader.feed('\r'));
  expectEvent(LineEvent::None, reader.feed('\r'));
}

static void test_max_length_line_is_accepted() {
  LineReader reader;
  for (int i = 0; i < 63; ++i) {
    expectEvent(LineEvent::None, reader.feed('a'));
  }
  expectEvent(LineEvent::Line, reader.feed('\n'));
  TEST_ASSERT_EQUAL_INT(63, static_cast<int>(strlen(reader.line())));
}

static void test_too_long_line_is_reported() {
  LineReader reader;
  for (int i = 0; i < 64; ++i) {
    expectEvent(LineEvent::None, reader.feed('a'));
  }
  expectEvent(LineEvent::TooLong, reader.feed('\n'));
}

static void test_next_line_replaces_previous() {
  LineReader reader;
  reader.feed('o');
  reader.feed('n');
  reader.feed('\n');
  reader.feed('o');
  reader.feed('f');
  reader.feed('f');
  reader.feed('\r');
  TEST_ASSERT_EQUAL_STRING("off", reader.line());
}

static void test_backspace_removes_last_char() {
  LineReader reader;
  reader.feed('o');
  reader.feed('n');
  reader.feed('x');
  TEST_ASSERT_TRUE(reader.backspace());
  expectEvent(LineEvent::Line, reader.feed('\n'));
  TEST_ASSERT_EQUAL_STRING("on", reader.line());
}

static void test_backspace_at_start_is_ignored() {
  LineReader reader;
  TEST_ASSERT_FALSE(reader.backspace());
}

static void test_backspace_does_not_fix_overflow() {
  LineReader reader;
  for (int i = 0; i < 64; ++i) {
    reader.feed('a');
  }
  TEST_ASSERT_FALSE(reader.backspace());
  expectEvent(LineEvent::TooLong, reader.feed('\n'));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_lf_ends_line);
  RUN_TEST(test_cr_ends_line);
  RUN_TEST(test_crlf_is_one_line);
  RUN_TEST(test_empty_lines_are_ignored);
  RUN_TEST(test_max_length_line_is_accepted);
  RUN_TEST(test_too_long_line_is_reported);
  RUN_TEST(test_next_line_replaces_previous);
  RUN_TEST(test_backspace_removes_last_char);
  RUN_TEST(test_backspace_at_start_is_ignored);
  RUN_TEST(test_backspace_does_not_fix_overflow);
  return UNITY_END();
}
