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
    return true;
}

bool mudouModoAC(MatterThermostat::ThermostatMode_t modo_novo) {
    acCmd::Command comando;
    switch(modo_novo) {
        case MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_AUTO:  // automatico
            comando.tipo = commandType::OpMode;
            comando.instrucao.modo_operacao = stdAc::opmode_t::kAuto;
            break;
        case MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_COOL:  // esfriar
            comando.tipo = commandType::OpMode;
            comando.instrucao.modo_operacao = stdAc::opmode_t::kCool;
            break;
        case MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_DRY:   // secar
            comando.tipo = commandType::OpMode;
            comando.instrucao.modo_operacao = stdAc::opmode_t::kDry;
            break;
        case MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_HEAT:  // aquecer
            comando.tipo = commandType::OpMode;
            comando.instrucao.modo_operacao = stdAc::opmode_t::kHeat;
            break;
        case MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_FAN_ONLY:  // ventilar
            comando.tipo = commandType::OpMode;
            comando.instrucao.modo_operacao = stdAc::opmode_t::kFan;
            break;
        case MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_OFF:   // desligar
            comando.tipo = commandType::Power;
            comando.instrucao.ligar = false;
            break;
        default:
            Serial.println("Comando desconhecido de opmode recebido do Matter!");
            return false;
    }
    loop_protocolos(&comando);
    return true;
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