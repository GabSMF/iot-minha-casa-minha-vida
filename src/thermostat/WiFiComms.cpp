#include "WiFiComms.h"

Preferences WiFiPreferences;

void recomissionarMatter() {
    if (!Matter.isDeviceCommissioned()) {
        Serial.print("Código de pareamento: ");
        Serial.println(Matter.getManualPairingCode());

        Serial.print("Site com QR code de pareamento: ");
        Serial.println(Matter.getOnboardingQRCodeUrl());

        Serial.print("Procurando ambiente Matter...");
        while (!Matter.isDeviceCommissioned()) {
            Serial.print(".");
            delay(1000);
        }
        Serial.println("\nConectado no ambiente Matter!");
    }
}

bool mudouTemperaturaAC(double temp_nova) {
    acCmd::Command comando;
    comando.tipo = commandType::Temperature;
    comando.instrucao.temperatura = temp_nova;
    loop_protocolos(&comando);
}

void reconectarWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        String name, password;
        name = WiFiPreferences.getString("wifiname", "Projeto");
        password = WiFiPreferences.getString("wifipass", "2022-11-07");
        WiFi.begin(name, password);

        Serial.println("Conectando ao WiFi...");
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print(".");
            delay(1000);
        }
        Serial.print("conectado!\nEndereço IP: ");
        Serial.println(WiFi.localIP());
    }
}