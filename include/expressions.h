#pragma once

#include <cstdint>

// The four faces the pal can wear, in knob order.
enum class Expression : uint8_t { Sleepy, Calm, Happy, Surprised };

constexpr uint8_t kExpressionCount = 4;

constexpr Expression nextExpression(Expression face, int8_t delta) {
  int index = (static_cast<int>(face) + delta) % kExpressionCount;
  return static_cast<Expression>(index < 0 ? index + kExpressionCount : index);
}

inline const char* expressionName(Expression face) {
  switch (face) {
    case Expression::Sleepy:
      return "sleepy";
    case Expression::Calm:
      return "calm";
    case Expression::Happy:
      return "happy";
    case Expression::Surprised:
      return "surprised";
  }
  return "calm";
}

// One note per face: C4, E4, G4, C5.
inline uint16_t expressionNoteHz(Expression face) {
  switch (face) {
    case Expression::Sleepy:
      return 262;
    case Expression::Calm:
      return 330;
    case Expression::Happy:
      return 392;
    case Expression::Surprised:
      return 523;
  }
  return 330;
}
