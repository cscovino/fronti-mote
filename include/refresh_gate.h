#pragma once

#include <cstdint>

// The panel flashes on every update, so never refresh the same face twice and
// never closer than this apart.
constexpr uint32_t kMinRefreshGapMs = 2000;

// How long to wait before refreshing, in ms; 0 means refresh now. Wrapping
// millis() differences stay correct across the 32-bit rollover.
inline uint32_t refreshWaitMs(bool haveShown, uint32_t nowMs, uint32_t lastMs) {
  if (!haveShown) return 0;
  const uint32_t since = nowMs - lastMs;
  return since < kMinRefreshGapMs ? kMinRefreshGapMs - since : 0;
}
