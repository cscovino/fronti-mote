#pragma once

#include <cstddef>
#include <cstring>

// Splits a byte stream into lines terminated by CR, LF or CRLF.
// Lines are trimmed of surrounding spaces; a blank line is not reported.
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
      while (len_ > 0 && buf_[len_ - 1] == ' ') --len_;
      size_t start = 0;
      while (start < len_ && buf_[start] == ' ') ++start;
      if (start > 0) {
        memmove(buf_, buf_ + start, len_ - start);
        len_ -= start;
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
