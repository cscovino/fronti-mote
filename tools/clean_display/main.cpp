#include <Arduino.h>
#include <GxEPD2_3C.h>

GxEPD2_3C<GxEPD2_154_Z90c, GxEPD2_154_Z90c::HEIGHT> display(
    GxEPD2_154_Z90c(D0, D1, D2, D9));

void setup() {
  display.init(0);
  display.fillScreen(GxEPD_WHITE);
  display.display(false);
  display.hibernate();
}

void loop() {}
