#include "MatterACEndpoint.h"

using namespace esp_matter;
using namespace esp_matter::cluster;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

stdAc::opmode_t convertOpmodeMatterToIRremote(uint8_t MatterOpmode) {
    switch(MatterOpmode) {
        case chip::to_underlying(Thermostat::SystemModeEnum::kOff):
            return stdAc::opmode_t::kOff;
        case chip::to_underlying(Thermostat::SystemModeEnum::kCool):
            return stdAc::opmode_t::kCool;
        case chip::to_underlying(Thermostat::SystemModeEnum::kDry):
            return stdAc::opmode_t::kDry;
        case chip::to_underlying(Thermostat::SystemModeEnum::kFanOnly):
            return stdAc::opmode_t::kFan;
        case chip::to_underlying(Thermostat::SystemModeEnum::kAuto):
            return stdAc::opmode_t::kCool;
        default:
            return stdAc::opmode_t::kAuto;  // Fallback
    }
}

uint8_t convertOpmodeIRremoteToMatter(stdAc::opmode_t IRopmode) {
    switch (IRopmode) {
        case stdAc::opmode_t::kOff:
            return chip::to_underlying(Thermostat::SystemModeEnum::kOff);
        case stdAc::opmode_t::kCool:
            return chip::to_underlying(Thermostat::SystemModeEnum::kCool);
        case stdAc::opmode_t::kHeat:
            return chip::to_underlying(Thermostat::SystemModeEnum::kHeat);
        case stdAc::opmode_t::kDry:
            return chip::to_underlying(Thermostat::SystemModeEnum::kDry);
        case stdAc::opmode_t::kFan:
            return chip::to_underlying(Thermostat::SystemModeEnum::kFanOnly);
        case stdAc::opmode_t::kAuto:
            return chip::to_underlying(Thermostat::SystemModeEnum::kAuto);
        default:
            return chip::to_underlying(Thermostat::SystemModeEnum::kAuto); // Fallback
    }
    return 0;
}

stdAc::fanspeed_t convertFanmodeMatterToIRremote(uint8_t MatterFanmode) {
    switch (MatterFanmode) {
        case chip::to_underlying(FanControl::FanModeEnum::kAuto):
            return stdAc::fanspeed_t::kAuto;
        case chip::to_underlying(FanControl::FanModeEnum::kLow):
            return stdAc::fanspeed_t::kLow;
        case chip::to_underlying(FanControl::FanModeEnum::kMedium):
            return stdAc::fanspeed_t::kMedium;
        case chip::to_underlying(FanControl::FanModeEnum::kHigh):
            return stdAc::fanspeed_t::kHigh;
        default:
            return stdAc::fanspeed_t::kAuto;    // Fallback
    }   
}

uint8_t convertFanmodeIRremoteToMatter(stdAc::fanspeed_t IRfanmode) {
    switch (IRfanmode) {
        case stdAc::fanspeed_t::kAuto:
            return chip::to_underlying(FanControl::FanModeEnum::kAuto);
        case stdAc::fanspeed_t::kMin:
            return chip::to_underlying(FanControl::FanModeEnum::kLow);
        case stdAc::fanspeed_t::kLow:
            return chip::to_underlying(FanControl::FanModeEnum::kLow);
        case stdAc::fanspeed_t::kMedium:
            return chip::to_underlying(FanControl::FanModeEnum::kMedium); 
        case stdAc::fanspeed_t::kHigh:
            return chip::to_underlying(FanControl::FanModeEnum::kHigh);
        case stdAc::fanspeed_t::kMax:
            return chip::to_underlying(FanControl::FanModeEnum::kHigh);
        default:
            return chip::to_underlying(FanControl::FanModeEnum::kAuto);     // Fallback
    }
    return 0;
}

MatterAC::MatterAC() {}

MatterAC::~MatterAC() {
    end();
}

bool MatterAC::addFanControlCluster() {
    fan_control::config_t fan_config;
    fan_config.fan_mode = _fanMode;
    fan_config.percent_setting = _fanSpeedPercent;
    _fan_control_cluster = fan_control::create(_matterEndpoint, &fan_config, CLUSTER_FLAG_SERVER);

    if (!_fan_control_cluster) {
        Serial.println("Falha ao criar cluster de ventilador.");
        return false;
    }
    return true;
}

bool MatterAC::begin() {
    if (getEndPointId() != 0 || started) {
        Serial.println("AC Matter já foi criado!");
        return false;
    }

    // Config básica
    room_air_conditioner::config_t ac_config;
    stdAc::state_t estado_atual = ar_condicionado.getState();
    ac_config.on_off.on_off = _isOn;
    ac_config.thermostat.cooling.occupied_cooling_setpoint = (uint16_t)(_currentTemperature * 100);
    ac_config.thermostat.heating.occupied_heating_setpoint = ac_config.thermostat.cooling.occupied_cooling_setpoint;
    ac_config.thermostat.system_mode = _thermostatMode;
    ac_config.thermostat.control_sequence_of_operation = chip::to_underlying(Thermostat::ControlSequenceOfOperationEnum::kCoolingAndHeating);

    // Criar endpoint Matter
    _matterEndpoint = room_air_conditioner::create(node::get(), &ac_config, ENDPOINT_FLAG_NONE, (void*) this);
    if (_matterEndpoint == nullptr) {
        Serial.println("Falha ao criar endpoint do AC");
        return false;
    }

    // Guardar ID do endpoint Matter na classe base
    setEndPointId(endpoint::get_id(_matterEndpoint));

    // Pega ponteiros para cada cluster criado por room_air_conditioner::create
    _on_off_cluster = cluster::get(_matterEndpoint, OnOff::Id);
    _thermostat_cluster = cluster::get(_matterEndpoint, Thermostat::Id);

    if (!_on_off_cluster || !_thermostat_cluster) {
        Serial.println("Falha em recuperar clusters após criação de endpoint.");
        return false;
    }

    // Ponteiro para temperatura local para updates futuros
    // _local_temp_attr = cluster::getAttribute(_thermostat_cluster, Thermostat);

    // Cria cluster de ventilador manualmente
    if (!addFanControlCluster()) {
        Serial.println("Falha ao criar cluster de ventilador.");
        return false;
    }

    // @todo: Sincronizar atributos

    Serial.println("Ar condicionado Matter criado com endpoint_id "+String(getEndPointId()));
    started = true;
    return true;
}

void MatterAC::end() {
    started = false;
}

bool MatterAC::attributeChangeCB(uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val) {
    if (endpoint_id != getEndPointId()) {
        Serial.println("Mudança de atributo para endpoint desconhecido "+String(endpoint_id));
        return false;
    }

    if (cluster_id == OnOff::Id) {
        if (attribute_id == OnOff::Attributes::OnOff::Id) {
            handleOnOffChange(val->val.b);
            return true;
        }
    }
    else if (cluster_id == Thermostat::Id) {
        handleThermostatChange(attribute_id, val);
        return true;
    }
    else if (cluster_id == FanControl::Id) {
        handleFanControlChange(attribute_id, val);
        return true;
    }
    return false;   // se chegou aqui, o cluster-alvo é desconhecido...
}

/*
    LIGA/DESLIGA.
*/
void MatterAC::handleOnOffChange(bool newState) {
    _isOn = newState;

    acCmd::Command comando;
    comando.tipo = commandType::Power;
    comando.instrucao.ligar = _isOn;

    loop_protocolos(&comando);  // ligar ou desligar ar condicionado!
}

/*
    VELOCIDADE DO VENTILADOR (FanMode <-> fanspeed_t);
    PORCENTAGEM DE VELOCIDADE.
*/
void MatterAC::handleFanControlChange(uint32_t attribute_id, esp_matter_attr_val_t *val) {
    if (attribute_id == FanControl::Attributes::FanMode::Id) {
        _fanMode = val->val.u8;
        stdAc::fanspeed_t vel_ventilador = convertFanmodeMatterToIRremote(_fanMode);

        acCmd::Command comando;
        comando.tipo = commandType::FanSpeed;
        comando.instrucao.vel_ventilador = vel_ventilador;

        loop_protocolos(&comando);  // mudar velocidade!
    }
    else if (attribute_id == FanControl::Attributes::PercentSetting::Id) {
        _fanSpeedPercent = val->val.u8;
        stdAc::fanspeed_t vel_ventilador;

        // Mapeando porcentagens de velocidade da Alexa para valores do Enum do Ar Condicionado.
        if (_fanSpeedPercent <= 1) {
            vel_ventilador = stdAc::fanspeed_t::kMin;
        }
        else if (_fanSpeedPercent <= 35) {
            vel_ventilador = stdAc::fanspeed_t::kLow;
        }
        else if (_fanSpeedPercent <= 65) {
            vel_ventilador = stdAc::fanspeed_t::kMedium;
        }
        else if (_fanSpeedPercent <= 99) {
            vel_ventilador = stdAc::fanspeed_t::kHigh;
        }
        else {
            vel_ventilador = stdAc::fanspeed_t::kMax;
        }

        acCmd::Command comando;
        comando.tipo = commandType::FanSpeed;
        comando.instrucao.vel_ventilador = vel_ventilador;

        loop_protocolos(&comando);  // mudar velocidade a partir de porcentagem!
    }
}

/*
    TEMPERATURA;
    MODO DE OPERAÇÃO DO AC.
*/
void MatterAC::handleThermostatChange(uint32_t attribute_id, esp_matter_attr_val_t *val) {
    if (attribute_id == Thermostat::Attributes::OccupiedCoolingSetpoint::Id ||
        attribute_id == Thermostat::Attributes::OccupiedHeatingSetpoint::Id) {  // O Matter diferencia Temp. de aquecimento e resfriamento.
        uint16_t temperatura_raw = val->val.u16;
        float temperatura_nova = temperatura_raw / 100.0f; 
        _currentTemperature = temperatura_nova;

        acCmd::Command comando;
        comando.tipo = commandType::Temperature;
        comando.instrucao.temperatura = temperatura_nova;

        loop_protocolos(&comando);  // mudar temperatura!!
    }
    else if (attribute_id == Thermostat::Attributes::SystemMode::Id) {
        uint8_t modo_raw = val->val.u8;
        _thermostatMode = modo_raw;
        stdAc::opmode_t modo_novo = convertOpmodeMatterToIRremote(modo_raw);

        acCmd::Command comando;
        comando.tipo = commandType::OpMode;
        comando.instrucao.modo_operacao = modo_novo;

        loop_protocolos(&comando);  // mudar modo de operação!
    }
}