#include "WiFiComms.h"

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