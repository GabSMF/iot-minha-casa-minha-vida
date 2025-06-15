#pragma once
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRutils.h>

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
IRac ar_condicionado(IRled);
