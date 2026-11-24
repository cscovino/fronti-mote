#pragma once

#include <Arduino.h>

#include "expressions.h"

// Panel wiring, shared with the clean-display tool.
constexpr uint8_t kPanelCsPin = D0;    // GPIO1
constexpr uint8_t kPanelDcPin = D1;    // GPIO2
constexpr uint8_t kPanelRstPin = D2;   // GPIO3
constexpr uint8_t kPanelBusyPin = D9;  // GPIO8, SPI MISO is unused

void displayBegin();
void displayShow(Expression face, uint8_t bands);
