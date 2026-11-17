#include "mqtt_link.h"

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <esp_timer.h>

#include "expressions.h"
#include "mqtt_command.h"
#include "pal.h"
#include "wifi_link.h"

#if __has_include("secrets.h")
#include "secrets.h"
#define FRONTIMOTE_SECRETS 1
#endif

namespace {

constexpr uint32_t kRetryIntervalMs = 5000;
constexpr size_t kTopicLen = 96;
constexpr size_t kStateLen = 192;

#ifdef FRONTIMOTE_SECRETS
constexpr const char* kHost = kMqttHost;
constexpr uint16_t kPort = kMqttPort;
constexpr const char* kUser = kMqttUser;
constexpr const char* kPassword = kMqttPassword;
constexpr const char* kPrefix = kMqttPrefix;
#else
constexpr const char* kHost = "";
constexpr uint16_t kPort = 1883;
constexpr const char* kUser = "";
constexpr const char* kPassword = "";
constexpr const char* kPrefix = "frontimote";
#endif

constexpr bool configured() { return kHost[0] != '\0'; }

WiFiClient net;
PubSubClient client(net);

char cmdTopic[kTopicLen];
char stateTopic[kTopicLen];
uint32_t lastAttemptMs = 0;
Expression publishedFace = Expression::Calm;

void publishState() {
  char payload[kStateLen];
  const Expression face = palFace();
  snprintf(payload, sizeof payload,
           "{\"online\":true,\"face\":\"%s\",\"rssi\":%d,\"heap\":%lu,"
           "\"uptime\":%llu}",
           expressionName(face), (int)WiFi.RSSI(),
           (unsigned long)ESP.getFreeHeap(),
           (unsigned long long)(esp_timer_get_time() / 1000000));
  // Remember the attempt even if it fails: the next face change or the
  // reconnect republish will try again without a per-tick retry storm.
  publishedFace = face;
  if (!client.publish(stateTopic, payload, true)) {
    Serial.println("mqtt   : state publish failed");
  }
}

void handlePayload(const char* payload, unsigned int length) {
  const PetCommand cmd = parsePetCommand(payload, length);
  if (!cmd.valid) {
    Serial.println("mqtt   : payload is not json");
    return;
  }

  bool acted = false;
  if (cmd.setFace) {
    palSetFace(cmd.face);
    acted = true;
  }
  if (cmd.nextFace) {
    palCycleFace();
    acted = true;
  }
  if (cmd.beep) {
    palBeep(cmd.beepHz);
    acted = true;
  }
  if (cmd.hasTune) {
    palPlayTune(cmd.tune);
    acted = true;
  }
  if (cmd.status) {
    publishState();
    acted = true;
  }

  if (!acted)
    Serial.printf("mqtt   : ignored \"%.*s\"\n", (int)length, payload);
}

void onMessage(char*, uint8_t* payload, unsigned int length) {
  handlePayload(reinterpret_cast<const char*>(payload), length);
}

void buildTopics() {
  snprintf(cmdTopic, sizeof cmdTopic, "%s/cmd", kPrefix);
  snprintf(stateTopic, sizeof stateTopic, "%s/state", kPrefix);
}

// The network stack caps a TCP connect at 3 s and the socket timeout below
// cuts a stalled read, so a sick broker can still pause loop() for seconds.
// shortcut: synchronous connect, move to an async client if the encoder
// ever stutters when the broker is down.
void connectBroker() {
  char clientId[32];
  snprintf(clientId, sizeof clientId, "frontimote-%06lx",
           (unsigned long)(ESP.getEfuseMac() & 0xFFFFFF));
  const bool hasCredentials = kUser[0] != '\0';

  if (!client.connect(clientId, hasCredentials ? kUser : nullptr,
                      hasCredentials ? kPassword : nullptr, stateTopic, 0, true,
                      "{\"online\":false}")) {
    Serial.printf("mqtt   : connect failed (state %d)\n", client.state());
    return;
  }

  Serial.printf("mqtt   : connected %s:%u\n", kHost, (unsigned)kPort);
  client.subscribe(cmdTopic);
  publishState();
}

}  // namespace

void mqttBegin() {
  if (!configured()) {
    Serial.println("mqtt   : not configured (set kMqttHost in secrets.h)");
    return;
  }

  buildTopics();
  client.setServer(kHost, kPort);
  client.setKeepAlive(15);
  client.setSocketTimeout(3);
  client.setBufferSize(512);
  client.setCallback(onMessage);
  Serial.printf("mqtt   : broker %s:%u, topic %s\n", kHost, (unsigned)kPort,
                cmdTopic);
}

void mqttTick() {
  if (!configured() || !wifiIsOnline()) return;

  if (client.connected()) {
    client.loop();
    if (palFace() != publishedFace) publishState();
    return;
  }

  const uint32_t now = millis();
  if (lastAttemptMs != 0 && now - lastAttemptMs < kRetryIntervalMs) return;
  lastAttemptMs = now;
  connectBroker();
}

void mqttPrintStatus() {
  if (!configured()) {
    Serial.println("mqtt   : not configured (set kMqttHost in secrets.h)");
    return;
  }
  if (client.connected()) {
    Serial.printf("mqtt   : connected %s:%u\n", kHost, (unsigned)kPort);
  } else if (!wifiIsOnline()) {
    Serial.println("mqtt   : waiting for wifi");
  } else {
    Serial.printf("mqtt   : connecting to %s:%u\n", kHost, (unsigned)kPort);
  }
}
