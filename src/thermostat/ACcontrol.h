#pragma once
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRutils.h>
#include <Preferences.h>

enum class commandType {
    Power,
    Temperature,
    OpMode,
    FanSpeed
};

union commandUnion {
    bool ligar;
    float temperatura;
    stdAc::opmode_t modo_operacao;
    stdAc::fanspeed_t vel_ventilador;
};

namespace acCmd {
    struct Command {
        commandType tipo;
        commandUnion instrucao;
    };
};

const uint16_t IRled = 4;
extern IRac ar_condicionado;
extern Preferences ACpreferences;

void setup_AC();
void loop_protocolos(acCmd::Command *comando);
void send_coolix(acCmd::Command *comando);
