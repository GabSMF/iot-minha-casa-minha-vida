#pragma once
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>

extern GxEPD2_290_T94_V2 modeloTela;
extern GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela;
extern U8G2_FOR_ADAFRUIT_GFX fontes;

void setup_EInk();