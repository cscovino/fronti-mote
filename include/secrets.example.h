#pragma once

// Copy to secrets.h (gitignored) and fill in. Empty password = open network.
// These strings are compiled into flash as plain text; use a dedicated network.
constexpr char kWifiSsid[] = "";
constexpr char kWifiPassword[] = "";

// Empty host leaves MQTT off; prefix is the root of the two topics.
// Give each unit on one broker its own prefix (e.g. frontimote-kitchen).
constexpr char kMqttHost[] = "";
constexpr uint16_t kMqttPort = 1883;
constexpr char kMqttUser[] = "";
constexpr char kMqttPassword[] = "";
constexpr char kMqttPrefix[] = "frontimote";
