#include <Arduino.h>
#include <GxEPD2_BW.h>

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(D0, D1,
                                                                         D2,
                                                                         D9));

void setup() {
  display.init(0);
  display.fillScreen(GxEPD_WHITE);
  display.display(false);
  display.hibernate();
}

void loop() {}
