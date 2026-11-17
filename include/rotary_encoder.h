#pragma once

#include <cstdint>

// Decodes a quadrature rotary encoder into -1, 0 or +1 per detent.
// Invalid transitions (both contacts changing at once) score 0, so bounce
// cancels itself out instead of producing phantom clicks.
class RotaryEncoder {
 public:
  // EC11-style: 4 quadrature steps per detent; use 2 if one click plays two
  // notes.
  static constexpr int8_t kStepsPerDetent = 4;

  int8_t update(bool a, bool b) {
    const uint8_t cur =
        static_cast<uint8_t>(a) | (static_cast<uint8_t>(b) << 1);
    // index = prev << 2 | cur, bits: A = 1, B = 2
    static constexpr int8_t kTable[16] = {0, 1, -1, 0,  -1, 0,  0, 1,
                                          1, 0, 0,  -1, 0,  -1, 1, 0};
    acc_ += kTable[(prev_ << 2) | cur];
    prev_ = cur;
    if (acc_ >= kStepsPerDetent) {
      acc_ -= kStepsPerDetent;
      return 1;
    }
    if (acc_ <= -kStepsPerDetent) {
      acc_ += kStepsPerDetent;
      return -1;
    }
    return 0;
  }

 private:
  uint8_t prev_ = 0;
  int8_t acc_ = 0;
};
