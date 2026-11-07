#pragma once

#include <cstddef>

// Splits a byte stream into lines terminated by CR, LF or CRLF.
// A line longer than 63 characters is discarded and reported as TooLong.
enum class LineEvent : unsigned char { None, Line, TooLong };

class LineReader {
 public:
  static constexpr size_t kCapacity = 64;

  LineEvent feed(char c) {
    if (c == '\r' || c == '\n') {
      if (tooLong_) {
        tooLong_ = false;
        len_ = 0;
        buf_[0] = '\0';
        return LineEvent::TooLong;
      }
      if (len_ == 0) {
        return LineEvent::None;
      }
      buf_[len_] = '\0';
      len_ = 0;
      return LineEvent::Line;
    }
    if (len_ < kCapacity - 1) {
      buf_[len_++] = c;
    } else {
      tooLong_ = true;
    }
    return LineEvent::None;
  }

  bool backspace() {
    if (tooLong_ || len_ == 0) {
      return false;
    }
    --len_;
    return true;
  }

  // Valid until the next feed().
  const char* line() const { return buf_; }

 private:
  char buf_[kCapacity] = {};
  size_t len_ = 0;
  bool tooLong_ = false;
};
