#pragma once

#include "expressions.h"

// Call palBegin() after Serial is up; it reports buzzer failures there.
void palBegin();
void palTick();
void palLed(bool on);
Expression palFace();
void palCycleFace();
void palBeep();
void palSilence();
