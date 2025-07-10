#include "WiFiComms.h"

MatterThermostat ac_matter;
Preferences WiFiPreferences;
WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1000);

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
    ac_matter.setLocalTemperature(temp_nova);
    acCmd::Command comando;
    comando.tipo = commandType::Temperature;
    comando.instrucao.temperatura = (float)temp_nova;
    loop_protocolos(&comando);
    changeDetectedCB();
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
    changeDetectedCB();
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

void setupSecureClient() {
    conexaoSegura.setCACert(certificado1);
}

void setupMQTT() {
    mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);
    mqtt.onMessage(recebeuMensagem);
    mqtt.setKeepAlive(10);
    mqtt.setWill("dead", "Desconectado por inatividade.");
    
    reconectarMQTT();
}

void reconectarMQTT() {
    if (!mqtt.connected()) {
        Serial.print("Conectando MQTT...");
        while (!mqtt.connected()) {
            mqtt.connect("THERMOSTAT", "aula", "zowmad-tavQez");
            Serial.print(".");
            delay(1000);
        }
        Serial.println(" conectado!");

        mqtt.subscribe("/updates");
        mqtt.subscribe("/preferencias");
    }
}

void recebeuMensagem(String topico, String conteudo) {
    Serial.println("topico");
    Serial.println(topico);
    Serial.println("conteudo");
    Serial.println(conteudo);
    if (topico == "/updates" && conteudo == "1") {
        Serial.println("Recebeu updates");
        mqtt.publish("id_dispositivo", "1");
        changeDetectedCB();       
    } 
    else if (topico == "/preferencias") {
        Serial.println("Recebeu preferencias e estou mudando!");
        JsonDocument dados;
        deserializeJson(dados, conteudo);

        Serial.println("Conteudo:");
        Serial.println(conteudo);

        if (dados[0]["id_dispositivo"] == 1) {
            stdAc::state_t estado_novo = ar_condicionado.next;
            estado_novo.power = true;

            // temperatura
            if (dados[0]["ar_temperatura"] != NULL) {
                float ar_temperatura = (float) dados[0]["ar_temperatura"];
                estado_novo.degrees = ar_temperatura;
                double tempdbl = (double) ar_temperatura;
                // avisando o matter
                ac_matter.setCoolingHeatingSetpoints(tempdbl, tempdbl);
                ac_matter.setLocalTemperature(tempdbl);
            }

            // modo
            if (dados[0]["ar_modo"] != NULL) {
                String ar_modo = String(dados[0]["ar_modo"]);
                if (ar_modo == "aquecimento") {
                    estado_novo.mode = stdAc::opmode_t::kHeat;
                    ac_matter.setMode(MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_HEAT);
                }
                else if (ar_modo == "desumidificação") {
                    estado_novo.mode = stdAc::opmode_t::kDry;
                    ac_matter.setMode(MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_DRY);
                }
                else if (ar_modo == "refrigeracao") {
                    estado_novo.mode = stdAc::opmode_t::kCool;
                    ac_matter.setMode(MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_COOL);
                }
                else if (ar_modo == "ventilacao") {
                    estado_novo.mode = stdAc::opmode_t::kFan;
                    ac_matter.setMode(MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_FAN_ONLY);
                }
                else if (ar_modo == "automatico") {
                    estado_novo.mode = stdAc::opmode_t::kAuto;
                    ac_matter.setMode(MatterThermostat::ThermostatMode_t::THERMOSTAT_MODE_AUTO);
                }
            }

            // fanspeed
            if (dados[0]["ar_velocidade"] != NULL) {
                String ar_velocidade = String(dados[0]["ar_velocidade"]);
                if (ar_velocidade == "fraca") {
                    estado_novo.fanspeed = stdAc::fanspeed_t::kMin;
                }
                else if (ar_velocidade == "media"){
                    estado_novo.fanspeed = stdAc::fanspeed_t::kMedium;
                }
                else if (ar_velocidade == "forte"){
                    estado_novo.fanspeed = stdAc::fanspeed_t::kMax;
                }
            }

            // ENVIA O COMANDO NESSA PORRA
            Serial.println("Enviando comando a um ar condicionado COOLIX");
            ar_condicionado.next = estado_novo;
            draw_current_state(&estado_novo);
            ar_condicionado.sendAc();
            changeDetectedCB();
        }
    }
}

// QUANDO RECEBER UPDATE DO SERVIDOR DE PREFERENCIAS
void changeDetectedCB() {
    JsonDocument jLeitura;
    String leitura;
    stdAc::state_t estado = ar_condicionado.getState();

    jLeitura["id_dispositivo"] = "1";
    jLeitura["temperaturaAr"] = estado.degrees;
    switch(estado.mode) {
        case stdAc::opmode_t::kAuto:
            jLeitura["modoAr"] = "auto";
            break;
        case stdAc::opmode_t::kDry:
            jLeitura["modoAr"] = "secar";
            break;
        case stdAc::opmode_t::kCool:
            jLeitura["modoAr"] = "resfriar";
            break;
        case stdAc::opmode_t::kHeat:
            jLeitura["modoAr"] = "aquecimento";
            break;
        case stdAc::opmode_t::kFan:
            jLeitura["modoAr"] = "ventilar";
            break;
        default:
            jLeitura["modoAr"] = "auto";
            break;
    }
    switch(estado.fanspeed) {
        case stdAc::fanspeed_t::kMin:
            jLeitura["velocidadeAr"] = "min";
            break;
        case stdAc::fanspeed_t::kHigh:
            jLeitura["velocidadeAr"] = "max";
            break;
        case stdAc::fanspeed_t::kMax:
            jLeitura["velocidadeAr"] = "max";
            break;
        case stdAc::fanspeed_t::kLow:
            jLeitura["velocidadeAr"] = "min";
            break;
        case stdAc::fanspeed_t::kMedium:
            jLeitura["velocidadeAr"] = "med";
            break;
        case stdAc::fanspeed_t::kMediumHigh:
            jLeitura["velocidadeAr"] = "med";
            break;
        case stdAc::fanspeed_t::kAuto:
            jLeitura["velocidadeAr"] = "auto";
            break;
        default:
            jLeitura["velocidadeAr"] = "auto";
            break;
    }
    jLeitura["powerAr"] = estado.power;

    serializeJson(jLeitura, leitura);
    mqtt.publish("/leituraAr", leitura);
}