#pragma once

#include <cstdint>

enum class LinkStatus { Connecting, Online, Offline };

// A drop must survive this long so a reconnect attempt is not reported.
constexpr uint32_t kLinkDebounceMs = 3000;

// A failed attempt is retried no faster than this.
constexpr uint32_t kWifiRetryIntervalMs = 10000;

// msInState is millis() minus the moment the current status was entered.
inline LinkStatus nextLinkStatus(LinkStatus cur, bool rawConnected,
                                 uint32_t msInState, uint32_t debounceMs) {
  if (rawConnected) return LinkStatus::Online;
  if (cur == LinkStatus::Offline) return LinkStatus::Offline;
  return msInState >= debounceMs ? LinkStatus::Offline : cur;
}

// Retry only after the previous attempt ended and the gap has passed.
inline bool wifiRetryDue(LinkStatus status, bool attemptDone, uint32_t nowMs,
                         uint32_t lastMs, uint32_t intervalMs) {
  return status != LinkStatus::Online && attemptDone &&
         nowMs - lastMs >= intervalMs;
}

// The plain thresholds for bands 1-4, in dBm; band 0 means "no link" and
// is set by the caller, never computed here.
constexpr int8_t kBandFloorDbm[4] = {-80, -72, -65, -55};

// A step up needs this much clearance above the new band's threshold;
// a step down happens the moment the old threshold is lost.
constexpr int8_t kBandHystDb = 4;

// The RSSI band for a fresh reading. cur 0 (coming online) adopts the
// plain reading; rising costs the hysteresis, falling is immediate.
inline uint8_t nextBand(uint8_t cur, int8_t rssiDbm) {
  uint8_t raw = 1;
  for (uint8_t b = 4; b >= 2; --b) {
    if (rssiDbm >= kBandFloorDbm[b - 1]) {
      raw = b;
      break;
    }
  }
  if (cur == 0 || raw <= cur) return raw;
  return rssiDbm >= kBandFloorDbm[raw - 1] + kBandHystDb ? raw : cur;
}
