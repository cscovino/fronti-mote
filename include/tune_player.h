#pragma once

#include <cstdint>

struct Note {
  uint16_t hz;
  uint16_t slotMs;
};

inline uint32_t tuneLengthMs(const Note* notes, uint8_t len) {
  uint32_t total = 0;
  for (uint8_t i = 0; i < len; ++i) total += notes[i].slotMs;
  return total;
}

// Hz to sound at elapsedMs; 0 in the silence after each note and past the end.
inline uint16_t tuneNoteHz(const Note* notes, uint8_t len, uint32_t elapsedMs,
                           uint16_t onMs) {
  uint32_t slotStart = 0;
  for (uint8_t i = 0; i < len; ++i) {
    const uint32_t slotEnd = slotStart + notes[i].slotMs;
    if (elapsedMs < slotEnd) {
      return elapsedMs - slotStart < onMs ? notes[i].hz : 0;
    }
    slotStart = slotEnd;
  }
  return 0;
}
