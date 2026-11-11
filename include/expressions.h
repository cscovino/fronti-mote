#pragma once

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

constexpr ExpressionInfo kExpressions[kExpressionCount] = {
    {"sleepy", 262}, {"calm", 330}, {"happy", 392}, {"surprised", 523}};

inline const char* expressionName(Expression face) {
  const uint8_t i = static_cast<uint8_t>(face);
  return i < kExpressionCount ? kExpressions[i].name : "calm";
}

inline uint16_t expressionNoteHz(Expression face) {
  const uint8_t i = static_cast<uint8_t>(face);
  return i < kExpressionCount ? kExpressions[i].hz : 330;
}
