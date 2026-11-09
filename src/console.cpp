#include "console.h"

#include <Arduino.h>

#include "expressions.h"
#include "line_reader.h"
#include "pal.h"

namespace {

constexpr uint32_t kStatusIntervalMs = 2000;

LineReader reader;
uint32_t lastStatusMs = 0;

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  on       - turn the onboard LED on (stops blinking)");
  Serial.println("  off      - turn the LED off and silence the buzzer");
  Serial.println("  face     - cycle to the next expression");
  Serial.println("  buzz     - play a 440 Hz test tone");
  Serial.println("  status   - print chip/heap/expression information");
  Serial.println("  help     - this text");
}

void printStatus() {
  const Expression face = palFace();
  Serial.printf("uptime : %lu s\n", millis() / 1000);
  Serial.printf("chip   : %s rev %u @ %lu MHz\n", ESP.getChipModel(),
                ESP.getChipRevision(), (unsigned long)getCpuFrequencyMhz());
  Serial.printf("flash  : %lu bytes\n", (unsigned long)ESP.getFlashChipSize());
  Serial.printf("heap   : %lu bytes free\n", (unsigned long)ESP.getFreeHeap());
  if (ESP.getPsramSize() > 0) {
    Serial.printf("psram  : %lu bytes free\n",
                  (unsigned long)ESP.getFreePsram());
  }
  Serial.printf("face   : %s (%u Hz)\n", expressionName(face),
                (unsigned)expressionNoteHz(face));
}

void handleCommand(const char* cmd) {
  if (strcmp(cmd, "on") == 0) {
    palLed(true);
    Serial.println("LED on");
  } else if (strcmp(cmd, "off") == 0) {
    palLed(false);
    palSilence();
    Serial.println("LED off");
  } else if (strcmp(cmd, "face") == 0) {
    palCycleFace();
  } else if (strcmp(cmd, "buzz") == 0) {
    palBeep();
    Serial.println("buzz 440 Hz");
  } else if (strcmp(cmd, "status") == 0) {
    printStatus();
  } else if (strcmp(cmd, "help") == 0) {
    printHelp();
  } else if (cmd[0] != '\0') {
    Serial.printf("Unknown command: %s (try 'help')\n", cmd);
  }
}

// Non-blocking serial console: echoes input and accepts CR, LF or CRLF.
void pollSerial() {
  while (Serial.available() > 0) {
    const char c = static_cast<char>(Serial.read());

    if (c == '\b' || c == 0x7F) {
      if (reader.backspace()) {
        Serial.print("\b \b");
      }
      continue;
    }

    const LineEvent event = reader.feed(c);
    if (event == LineEvent::Line) {
      Serial.println();
      handleCommand(reader.line());
    } else if (event == LineEvent::TooLong) {
      Serial.println();
      Serial.println("Line too long (max 63 chars)");
    } else if (c != '\r' && c != '\n') {
      Serial.write(static_cast<uint8_t>(c));
    }
  }
}

}  // namespace

void consoleBegin() {
  Serial.begin(115200);
  // Never block loop() on a stalled USB host; drop log lines instead.
  Serial.setTxTimeoutMs(0);
  // Native USB CDC re-enumerates on boot; give the host a moment to attach.
  const uint32_t start = millis();
  while (!Serial && millis() - start < 2000) {
    delay(10);
  }
  Serial.println();
  Serial.println("FrontiMote starting...");
}

void consoleWelcome() {
  printStatus();
  printHelp();
}

void consoleTick() {
  pollSerial();

  const uint32_t now = millis();
  if (now - lastStatusMs >= kStatusIntervalMs) {
    lastStatusMs = now;
    Serial.printf("[%lu s] alive, heap=%lu\n", now / 1000,
                  (unsigned long)ESP.getFreeHeap());
  }
}
