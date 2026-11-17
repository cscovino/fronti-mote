#include <unity.h>

#include "expressions.h"

void setUp() {}
void tearDown() {}

static int asInt(Expression face) { return static_cast<int>(face); }

static void test_next_wraps_forward() {
  TEST_ASSERT_EQUAL_INT(asInt(Expression::Calm),
                        asInt(nextExpression(Expression::Sleepy, 1)));
  TEST_ASSERT_EQUAL_INT(asInt(Expression::Happy),
                        asInt(nextExpression(Expression::Calm, 1)));
  TEST_ASSERT_EQUAL_INT(asInt(Expression::Sleepy),
                        asInt(nextExpression(Expression::Surprised, 1)));
}

static void test_next_wraps_backward() {
  TEST_ASSERT_EQUAL_INT(asInt(Expression::Surprised),
                        asInt(nextExpression(Expression::Sleepy, -1)));
  TEST_ASSERT_EQUAL_INT(asInt(Expression::Sleepy),
                        asInt(nextExpression(Expression::Calm, -1)));
}

static void test_names_are_distinct() {
  TEST_ASSERT_EQUAL_STRING("sleepy", expressionName(Expression::Sleepy));
  TEST_ASSERT_EQUAL_STRING("calm", expressionName(Expression::Calm));
  TEST_ASSERT_EQUAL_STRING("happy", expressionName(Expression::Happy));
  TEST_ASSERT_EQUAL_STRING("surprised", expressionName(Expression::Surprised));
}

static void test_notes_rise_pitch() {
  TEST_ASSERT_EQUAL_UINT16(262, expressionNoteHz(Expression::Sleepy));
  TEST_ASSERT_EQUAL_UINT16(330, expressionNoteHz(Expression::Calm));
  TEST_ASSERT_EQUAL_UINT16(392, expressionNoteHz(Expression::Happy));
  TEST_ASSERT_EQUAL_UINT16(523, expressionNoteHz(Expression::Surprised));
}

static void test_expression_from_name() {
  Expression face = Expression::Surprised;
  TEST_ASSERT_TRUE(expressionFromName("happy", &face));
  TEST_ASSERT_EQUAL_INT(asInt(Expression::Happy), asInt(face));
  TEST_ASSERT_TRUE(expressionFromName("SLEEPY", &face));
  TEST_ASSERT_EQUAL_INT(asInt(Expression::Sleepy), asInt(face));

  face = Expression::Calm;
  TEST_ASSERT_FALSE(expressionFromName("next", &face));
  TEST_ASSERT_FALSE(expressionFromName("", &face));
  TEST_ASSERT_EQUAL_INT(asInt(Expression::Calm), asInt(face));
}

int main(int, char**) {
  UNITY_BEGIN();
  RUN_TEST(test_next_wraps_forward);
  RUN_TEST(test_next_wraps_backward);
  RUN_TEST(test_names_are_distinct);
  RUN_TEST(test_notes_rise_pitch);
  RUN_TEST(test_expression_from_name);
  return UNITY_END();
}
