#pragma once

#include "expressions.h"
#include "link_state.h"
#include "tunes.h"

// Call palBegin() after Serial is up; it reports buzzer failures there.
void palBegin();
void palTick();
void palLinkStatus(LinkStatus status);
void palLinkBand(uint8_t bands);
void palLinkUp();
void palLinkDown();
void palPlayTune(Tune tune);
Expression palFace();
void palSetFace(Expression face);
void palCycleFace();
void palBeep(uint16_t hz = 440);
