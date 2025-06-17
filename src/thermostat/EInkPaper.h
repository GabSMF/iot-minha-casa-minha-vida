#pragma once
#include "ACcontrol.h"
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

extern GxEPD2_290_T94_V2 modeloTela;
extern GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela;
extern U8G2_FOR_ADAFRUIT_GFX fontes;

void draw_borders();
void setup_EInk();
void draw_fan_speed(stdAc::fanspeed_t vel);
void draw_current_state(stdAc::state_t *estado);
