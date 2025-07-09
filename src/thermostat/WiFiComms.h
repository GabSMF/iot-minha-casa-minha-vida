#pragma once

#include "ACcontrol.h"
#include "certificados.h"
#include <Matter.h>
#include <MQTT.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

extern Preferences WiFiPreferences; 
extern WiFiClientSecure conexaoSegura;
extern MQTTClient mqtt;

// Matter
void recomissionarMatter();
bool mudouTemperaturaAC(double temp_nova);
bool mudouModoAC(MatterThermostat::ThermostatMode_t modo_novo);

// WiFi
void reconectarWiFi();

// HTTPS Secure Client
void setupSecureClient();

// MQTT
void setupMQTT();
void reconectarMQTT();
void recebeuMensagem(String topico, String conteudo);