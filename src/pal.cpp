#include "pal.h"

#include <Arduino.h>

#include "display.h"
#include "rotary_encoder.h"
#include "tune_player.h"
#include "tunes.h"

namespace {

// Onboard user LED: GPIO21 on the XIAO ESP32S3, active LOW (LOW = on).
constexpr uint8_t kLedPin = LED_BUILTIN;

// Encoder A/B on D4/D5; passive buzzer on D3 driven through a BC547B.
constexpr uint8_t kEncoderAPin = D4;
constexpr uint8_t kEncoderBPin = D5;
constexpr uint8_t kBuzzerPin = D3;
constexpr uint8_t kBuzzerBits = 10;

constexpr uint32_t kFaceSettleMs = 400;
constexpr uint16_t kTestHz = 440;
constexpr uint16_t kNoteMs = 200;

bool ledOn = false;
LinkStatus linkStatus = LinkStatus::Connecting;
uint32_t linkSinceMs = 0;

const Note* tune = nullptr;
uint8_t tuneLen = 0;
uint32_t tuneStartMs = 0;
uint16_t tuneHz = 0;
uint16_t tuneOnMs = 0;

RotaryEncoder encoder;
Expression face = Expression::Calm;
bool facePending = false;
uint32_t faceChangeMs = 0;

void playTune(const Note* notes, uint8_t len, uint16_t onMs) {
  tune = notes;
  tuneLen = len;
  tuneOnMs = onMs;
  tuneStartMs = millis();
  tuneHz = notes[0].hz;
  ledcWriteTone(kBuzzerPin, tuneHz);
}

void playNote(uint16_t hz) {
  static Note one;
  one = {hz, kNoteMs};
  playTune(&one, 1, kNoteMs);
}

void stopTune() {
  tune = nullptr;
  tuneHz = 0;
  ledcWriteTone(kBuzzerPin, 0);
}

void selectFace(Expression next) {
  face = next;
  facePending = true;
  faceChangeMs = millis();
  playNote(expressionNoteHz(face));
  Serial.printf("face: %s\n", expressionName(face));
}

void pollEncoder() {
  const int8_t delta = encoder.update(digitalRead(kEncoderAPin) == HIGH,
                                      digitalRead(kEncoderBPin) == HIGH);
  if (delta != 0) {
    selectFace(nextExpression(face, delta));
  }
}

}  // namespace

void palBegin() {
  pinMode(kLedPin, OUTPUT);
  digitalWrite(kLedPin, HIGH);

  pinMode(kEncoderAPin, INPUT_PULLUP);
  pinMode(kEncoderBPin, INPUT_PULLUP);
  if (!ledcAttach(kBuzzerPin, kTestHz, kBuzzerBits)) {
    Serial.println("buzzer : LEDC attach failed");
  }
  stopTune();

  displayBegin();
  displayShow(face);
}

void palTick() {
  const uint32_t now = millis();

  const bool wantLed = ledOnForStatus(linkStatus, now - linkSinceMs);
  if (wantLed != ledOn) {
    ledOn = wantLed;
    digitalWrite(kLedPin, wantLed ? LOW : HIGH);
  }

  if (tune) {
    const uint32_t elapsed = now - tuneStartMs;
    const uint16_t hz = tuneNoteHz(tune, tuneLen, elapsed, tuneOnMs);
    if (hz != tuneHz) {
      tuneHz = hz;
      ledcWriteTone(kBuzzerPin, hz);
    }
    if (elapsed >= tuneLengthMs(tune, tuneLen)) stopTune();
  }

  if (facePending && now - faceChangeMs >= kFaceSettleMs) {
    facePending = false;
    displayShow(face);
  }

  pollEncoder();
}

void palLinkStatus(LinkStatus status) {
  if (status == linkStatus) return;
  linkStatus = status;
  linkSinceMs = millis();
}

void palLinkUp() { palPlayTune(Tune::Up); }

void palLinkDown() { palPlayTune(Tune::Down); }

void palPlayTune(Tune tune) {
  const TuneInfo& info = tuneInfo(tune);
  playTune(info.notes, info.len, info.onMs);
}

Expression palFace() { return face; }

void palSetFace(Expression face) { selectFace(face); }

void palCycleFace() { selectFace(nextExpression(face, 1)); }

void palBeep(uint16_t hz) { playNote(hz); }
