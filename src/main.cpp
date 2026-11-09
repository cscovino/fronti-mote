#include <Arduino.h>

#include "console.h"
#include "pal.h"

void setup() {
  consoleBegin();
  palBegin();
  consoleWelcome();
}

void loop() {
  palTick();
  consoleTick();
}
