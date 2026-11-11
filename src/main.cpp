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
  vTaskDelay(1);  // let the idle task run; 1 ms is below every poll interval
}
