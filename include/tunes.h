#pragma once

#include <strings.h>

#include <cstdint>

#include "tune_player.h"

// Every tune the pal can play; the MQTT payload names them.
enum class Tune : uint8_t { Up, Down, Happy, Sad, Curious, Sleepy };

// Table order matches the enum, so tuneInfo() can index it directly.
struct TuneInfo {
  const char* name;
  const Note* notes;
  uint8_t len;
  uint16_t onMs;
};

constexpr Note kTuneUp[] = {{523, 160}, {659, 160}, {784, 160}};
constexpr Note kTuneDown[] = {{392, 300}, {294, 420}};
constexpr Note kTuneHappy[] = {{523, 140}, {659, 140}, {784, 140}, {1047, 260}};
constexpr Note kTuneSad[] = {{392, 320}, {330, 320}, {262, 480}};
constexpr Note kTuneCurious[] = {{659, 160}, {784, 160}};
constexpr Note kTuneSleepy[] = {{262, 420}, {330, 560}};

constexpr TuneInfo kTunes[] = {
    {"up", kTuneUp, 3, 80},
    {"down", kTuneDown, 2, 80},
    {"happy", kTuneHappy, 4, 100},
    {"sad", kTuneSad, 3, 180},
    {"curious", kTuneCurious, 2, 100},
    {"sleepy", kTuneSleepy, 2, 240},
};

constexpr uint8_t kTuneCount =
    static_cast<uint8_t>(sizeof(kTunes) / sizeof(kTunes[0]));
static_assert(static_cast<uint8_t>(Tune::Sleepy) + 1 == kTuneCount,
              "add the tune to the enum and kTunes together");

// Case-insensitive tune lookup; leaves *out alone on a miss.
inline bool tuneFromName(const char* name, Tune* out) {
  for (uint8_t i = 0; i < kTuneCount; ++i) {
    if (strcasecmp(name, kTunes[i].name) == 0) {
      *out = static_cast<Tune>(i);
      return true;
    }
  }
  return false;
}

inline const TuneInfo& tuneInfo(Tune tune) {
  const uint8_t i = static_cast<uint8_t>(tune);
  return kTunes[i < kTuneCount ? i : 0];
}
