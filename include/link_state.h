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

// Also restarts the blink phase, so a change is visible at once.
inline bool ledOnForStatus(LinkStatus status, uint32_t msInState) {
  switch (status) {
    case LinkStatus::Online:
      return true;
    case LinkStatus::Connecting:
      return (msInState / 200) % 2 == 0;
    case LinkStatus::Offline:
      return (msInState / 1000) % 2 == 0;
  }
  return false;
}
