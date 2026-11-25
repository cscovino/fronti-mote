#include "wifi_link.h"

#include <Arduino.h>
#include <WiFi.h>

#include "link_state.h"
#include "pal.h"

#if __has_include("secrets.h")
#include "secrets.h"
#define FRONTIMOTE_SECRETS 1
#endif

namespace {

#ifdef FRONTIMOTE_SECRETS
constexpr const char* kSsid = kWifiSsid;
constexpr const char* kPassword = kWifiPassword;
#else
constexpr const char* kSsid = "";
constexpr const char* kPassword = "";
#endif

constexpr bool configured() { return kSsid[0] != '\0'; }

LinkStatus linkStatus = LinkStatus::Connecting;
uint32_t linkSinceMs = 0;
uint32_t lastRetryMs = 0;
bool everOnline = false;
// Written by the Wi-Fi event task, read by the loop.
volatile bool attemptDone = false;
// A console-initiated async scan is pending; wifiTick() collects it.
bool scanPending = false;

constexpr uint32_t kBandSampleMs = 15000;

uint8_t linkBand = 0;
uint32_t lastBandSampleMs = 0;

void reportBand(uint8_t band) {
  if (band == linkBand) return;
  linkBand = band;
  palLinkBand(band);
}

void sampleBand() {
  lastBandSampleMs = millis();
  reportBand(nextBand(linkBand, static_cast<int8_t>(WiFi.RSSI())));
}

const char* wlName(wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
    default:
      return "WL_UNKNOWN";
  }
}

void enter(LinkStatus next, uint32_t now) {
  linkStatus = next;
  linkSinceMs = now;
  palLinkStatus(next);
  if (next != LinkStatus::Online) reportBand(0);
}

void printConnected() {
  Serial.printf("wifi   : connected %s %d dBm\n",
                WiFi.localIP().toString().c_str(), (int)WiFi.RSSI());
}

void onStaDisconnect(arduino_event_id_t, arduino_event_info_t info) {
  attemptDone = true;
  const uint8_t reason = info.wifi_sta_disconnected.reason;
  Serial.printf("wifi   : disconnect reason %u (%s)\n", reason,
                WiFi.STA.disconnectReasonName((wifi_err_reason_t)reason));
}

void printScanResults(int count) {
  if (count < 0) {
    Serial.printf("wifi   : scan failed (%d)\n", count);
    return;
  }
  Serial.printf("wifi   : found %d networks\n", count);
  bool found = false;
  for (int i = 0; i < count; ++i) {
    const bool match = WiFi.SSID(i) == kSsid;
    found = found || match;
    Serial.printf("wifi   : %c \"%s\" ch %d %d dBm\n", match ? '*' : ' ',
                  WiFi.SSID(i).c_str(), (int)WiFi.channel(i),
                  (int)WiFi.RSSI(i));
  }
  if (!found) {
    Serial.printf("wifi   : \"%s\" not in the scan (2.4 GHz only)\n", kSsid);
  }
  WiFi.scanDelete();
}

// Blocks a few seconds at boot: it is the only way to see the air from here.
// A console scan runs async instead, so loop() keeps serving the pal.
void scanNetworks() {
  Serial.println("wifi   : scanning...");
  printScanResults(WiFi.scanNetworks(false, true));  // include hidden SSIDs
}

}  // namespace

bool wifiIsOnline() { return linkStatus == LinkStatus::Online; }

void wifiBegin() {
  if (!configured()) {
    enter(LinkStatus::Offline, millis());
    Serial.println(
        "wifi   : not configured (copy include/secrets.example.h to "
        "include/secrets.h)");
    return;
  }

  WiFi.mode(WIFI_STA);
  char hostname[24];
  snprintf(hostname, sizeof hostname, "frontimote-%06lx",
           (unsigned long)(ESP.getEfuseMac() & 0xFFFFFF));
  WiFi.setHostname(hostname);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(false);
  WiFi.onEvent(onStaDisconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  Serial.printf("wifi   : ssid \"%s\" (%u chars), password %u chars\n", kSsid,
                (unsigned)strlen(kSsid), (unsigned)strlen(kPassword));
  scanNetworks();
  attemptDone = false;
  WiFi.begin(kSsid, kPassword);
  enter(LinkStatus::Connecting, millis());
  Serial.printf("wifi   : connecting to \"%s\"\n", kSsid);
}

void wifiTick() {
  if (scanPending) {
    const int result = WiFi.scanComplete();
    if (result != WIFI_SCAN_RUNNING) {
      scanPending = false;
      printScanResults(result);
    }
  }

  if (!configured()) return;

  const uint32_t now = millis();
  const LinkStatus next =
      nextLinkStatus(linkStatus, WiFi.status() == WL_CONNECTED,
                     now - linkSinceMs, kLinkDebounceMs);
  if (next != linkStatus) {
    enter(next, now);
    if (next == LinkStatus::Online) {
      everOnline = true;
      attemptDone = false;
      palLinkUp();
      sampleBand();
      printConnected();
    } else if (next == LinkStatus::Offline) {
      if (everOnline) palLinkDown();
      Serial.printf("wifi   : offline (%s)\n", wlName(WiFi.status()));
    }
  }

  if (linkStatus == LinkStatus::Online &&
      now - lastBandSampleMs >= kBandSampleMs) {
    sampleBand();
  }

  if (wifiRetryDue(linkStatus, attemptDone, now, lastRetryMs,
                   kWifiRetryIntervalMs)) {
    attemptDone = false;
    lastRetryMs = now;
    WiFi.reconnect();
  }
}

void wifiPrintStatus() {
  if (!configured()) {
    Serial.println(
        "wifi   : not configured (copy include/secrets.example.h to "
        "include/secrets.h)");
    return;
  }

  switch (linkStatus) {
    case LinkStatus::Online:
      printConnected();
      break;
    case LinkStatus::Connecting:
      Serial.println("wifi   : connecting");
      break;
    case LinkStatus::Offline:
      Serial.printf("wifi   : offline (%s)\n", wlName(WiFi.status()));
      break;
  }
}

void wifiScan() {
  wifiPrintStatus();
  if (!configured() || scanPending) return;
  Serial.println("wifi   : scanning...");
  WiFi.scanNetworks(true, true);  // async, include hidden SSIDs
  scanPending = true;
}
