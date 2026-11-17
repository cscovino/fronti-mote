#pragma once

#include <ArduinoJson.h>

#include "expressions.h"
#include "tunes.h"

constexpr uint16_t kBeepMinHz = 100;
constexpr uint16_t kBeepMaxHz = 5000;

// What one MQTT payload asks for. Absent or invalid fields stay false.
struct PetCommand {
  bool valid = false;     // the payload was JSON
  bool setFace = false;   // face holds a known expression
  bool nextFace = false;  // the payload asked for the next expression
  Expression face = Expression::Calm;
  bool beep = false;  // beepHz is in range
  uint16_t beepHz = 0;
  bool hasTune = false;  // tune holds a known tune
  Tune tune = Tune::Up;
  bool status = false;
};

inline PetCommand parsePetCommand(const char* payload, size_t length) {
  PetCommand cmd;
  JsonDocument doc;
  if (deserializeJson(doc, payload, length)) return cmd;
  cmd.valid = true;

  const char* face = doc["face"] | "";
  if (face[0] != '\0') {
    if (strcasecmp(face, "next") == 0) {
      cmd.nextFace = true;
    } else {
      cmd.setFace = expressionFromName(face, &cmd.face);
    }
  }

  if (!doc["beep"].isNull()) {
    const long hz = doc["beep"].as<long>();
    if (hz >= kBeepMinHz && hz <= kBeepMaxHz) {
      cmd.beep = true;
      cmd.beepHz = static_cast<uint16_t>(hz);
    }
  }

  const char* tune = doc["tune"] | "";
  if (tune[0] != '\0') cmd.hasTune = tuneFromName(tune, &cmd.tune);

  if (doc["status"].as<bool>()) cmd.status = true;

  return cmd;
}
