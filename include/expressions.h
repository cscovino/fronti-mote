#pragma once

#include <strings.h>

#include <cstdint>

// The four faces the pal can wear, in knob order.
enum class Expression : uint8_t { Sleepy, Calm, Happy, Surprised };

constexpr uint8_t kExpressionCount = 4;

constexpr Expression nextExpression(Expression face, int8_t delta) {
  int index = (static_cast<int>(face) + delta) % kExpressionCount;
  return static_cast<Expression>(index < 0 ? index + kExpressionCount : index);
}

// One row per face: note name and pitch (C4, E4, G4, C5).
struct ExpressionInfo {
  const char* name;
  uint16_t hz;
};

constexpr ExpressionInfo kExpressions[] = {
    {"sleepy", 262}, {"calm", 330}, {"happy", 392}, {"surprised", 523}};
static_assert(sizeof(kExpressions) / sizeof(kExpressions[0]) ==
                  kExpressionCount,
              "kExpressionCount must match kExpressions");
static_assert(static_cast<uint8_t>(Expression::Surprised) + 1 ==
                  kExpressionCount,
              "add the face to the enum, kExpressions and kExpressionCount");

inline const char* expressionName(Expression face) {
  const uint8_t i = static_cast<uint8_t>(face);
  return i < kExpressionCount ? kExpressions[i].name : "calm";
}

// Case-insensitive lookup of a face by name; leaves *out alone on a miss.
inline bool expressionFromName(const char* name, Expression* out) {
  for (uint8_t i = 0; i < kExpressionCount; ++i) {
    if (strcasecmp(name, kExpressions[i].name) == 0) {
      *out = static_cast<Expression>(i);
      return true;
    }
  }
  return false;
}

inline uint16_t expressionNoteHz(Expression face) {
  const uint8_t i = static_cast<uint8_t>(face);
  return i < kExpressionCount ? kExpressions[i].hz : 330;
}
