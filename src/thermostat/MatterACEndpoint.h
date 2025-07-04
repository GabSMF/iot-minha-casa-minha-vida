#pragma once
#include <sdkconfig.h>
#ifdef CONFIG_ESP_MATTER_ENABLE_DATA_MODEL

#include <Arduino.h>
#include <Matter.h>
#include <MatterEndPoint.h>
#include "ACcontrol.h"

// Funções de conversão.
stdAc::opmode_t convertOpmodeMatterToIRremote(uint8_t MatterOpmode);
uint8_t convertOpmodeIRremoteToMatter(stdAc::opmode_t IRopmode);
stdAc::fanspeed_t convertFanmodeMatterToIRremote(uint8_t MatterFanmode);
uint8_t convertFanmodeIRremoteToMatter(stdAc::fanspeed_t IRfanmode);

class MatterAC : public MatterEndPoint {
public:
    MatterAC();     // construtor
    ~MatterAC();    // destrutor

    virtual bool begin(); 
    void end();

    bool attributeChangeCB(     // Chamada quando atributos mudam
        uint16_t endpoint_id,
        uint32_t cluster_id,
        uint32_t attribute_id,
        esp_matter_attr_val_t *val
    );      

protected:
    bool started = false;   // objeto matter ativo? y/n

    // Variáveis locais que rastreiam o estado atual do AC.
    bool _isOn = false;
    float _currentTemperature = 22.0; // Current ambient temperature
    uint8_t _thermostatMode = chip::to_underlying(Thermostat::SystemModeEnum::kOff); // Matter mode
    uint8_t _fanMode = chip::to_underlying(FanControl::FanModeEnum::kAuto); // Matter fan mode
    uint8_t _fanSpeedPercent = 0; // Matter fan speed percentage

    // Endpoint, cluster, e atributos
    esp_matter::endpoint_t *_matterEndpoint = nullptr;
    esp_matter::cluster_t *_on_off_cluster = nullptr;
    esp_matter::cluster_t *_thermostat_cluster = nullptr;
    esp_matter::cluster_t *_fan_control_cluster = nullptr;
    esp_matter::attribute_t *_local_temp_attr = nullptr;

    // Métodos, handlers de atualizações, etc
    bool addFanControlCluster();
    void handleOnOffChange(bool newState);
    void handleThermostatChange(uint32_t attribute_id, esp_matter_attr_val_t *val); 
    void handleFanControlChange(uint32_t attribute_id, esp_matter_attr_val_t *val);
};

#endif // CONFIG_ESP_MATTER_ENABLE_DATA_MODEL
