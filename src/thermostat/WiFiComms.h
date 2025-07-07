#pragma once

#include <Matter.h>
#include <Preferences.h>
#include <WiFi.h>

extern Preferences WiFiPreferences; 

// Matter
void recomissionarMatter();

// WiFi
void reconectarWiFi();