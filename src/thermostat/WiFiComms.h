#pragma once

#include <Matter.h>
#include <Preferences.h>
#include <WiFi.h>

extern Preferences WiFiPreferences; 

// Matter
void recomissionarMatter();
bool mudouTemperaturaAC(double temp_nova);
bool mudouModoAC(MatterThermostat::ThermostatMode_t modo_novo);

// WiFi
void reconectarWiFi();