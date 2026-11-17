#include <Arduino.h>

#include "console.h"
#include "mqtt_link.h"
#include "pal.h"
#include "wifi_link.h"

void setup() {
  consoleBegin();
  palBegin();
  wifiBegin();
  mqttBegin();
  consoleWelcome();
}

void loop() {
  palTick();
  wifiTick();
  mqttTick();
  consoleTick();
  vTaskDelay(1);  // let the idle task run; 1 ms is below every poll interval
}
