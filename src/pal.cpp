#include "pal.h"

#include <Arduino.h>

#include "display.h"
#include "rotary_encoder.h"

namespace {

// Onboard user LED: GPIO21 on the XIAO ESP32S3, active LOW (LOW = on).
constexpr uint8_t kLedPin = LED_BUILTIN;

// Encoder A/B on D4/D5; passive buzzer on D3 driven through a BC547B.
constexpr uint8_t kEncoderAPin = D4;
constexpr uint8_t kEncoderBPin = D5;
constexpr uint8_t kBuzzerPin = D3;
constexpr uint8_t kBuzzerBits = 10;

constexpr uint32_t kBlinkIntervalMs = 500;
constexpr uint32_t kFaceSettleMs = 400;
constexpr uint32_t kTestHz = 440;
constexpr uint32_t kNoteMs = 200;

bool ledOn = false;
bool autoBlink = true;
uint32_t lastBlinkMs = 0;

RotaryEncoder encoder;
Expression face = Expression::Calm;
bool facePending = false;
uint32_t faceChangeMs = 0;

bool notePlaying = false;
uint32_t noteStartMs = 0;

void setLed(bool on) {
  ledOn = on;
  digitalWrite(kLedPin, on ? LOW : HIGH);
}

void playTone(uint32_t hz) {
  ledcWriteTone(kBuzzerPin, hz);
  noteStartMs = millis();
  notePlaying = true;
}

void selectFace(Expression next) {
  face = next;
  facePending = true;
  faceChangeMs = millis();
  playTone(expressionNoteHz(face));
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
  setLed(false);

  pinMode(kEncoderAPin, INPUT_PULLUP);
  pinMode(kEncoderBPin, INPUT_PULLUP);
  if (!ledcAttach(kBuzzerPin, kTestHz, kBuzzerBits)) {
    Serial.println("buzzer : LEDC attach failed");
  }
  palSilence();

  displayBegin();
  displayShow(face);
}

void palTick() {
  const uint32_t now = millis();

  if (autoBlink && now - lastBlinkMs >= kBlinkIntervalMs) {
    lastBlinkMs = now;
    setLed(!ledOn);
  }

  if (notePlaying && now - noteStartMs >= kNoteMs) {
    palSilence();
  }

  if (facePending && now - faceChangeMs >= kFaceSettleMs) {
    facePending = false;
    displayShow(face);
  }

  pollEncoder();
}

void palLed(bool on) {
  autoBlink = false;
  setLed(on);
}

Expression palFace() { return face; }

void palCycleFace() { selectFace(nextExpression(face, 1)); }

void palBeep() { playTone(kTestHz); }

void palSilence() {
  ledcWriteTone(kBuzzerPin, 0);
  notePlaying = false;
}
