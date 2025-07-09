#include <Arduino.h>
#include <GFButton.h>
#include "ACcontrol.h"
#include "EInkPaper.h"
#include "WiFiComms.h"

unsigned long antes, agora = 0;
stdAc::state_t estado;

GFButton botao(48);

MatterThermostat ac_matter;

void botaoApertado(GFButton& botao) {
    Serial.println("Olá! Fui apertado!");
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    botao.setPressHandler(botaoApertado);

    ACpreferences.begin("ac_pref");
    WiFiPreferences.begin("wifi_pref");

    reconectarWiFi();
    
    setup_AC();
    setup_EInk();

    ac_matter.onChangeCoolingSetpoint(mudouTemperaturaAC);
    ac_matter.onChangeHeatingSetpoint(mudouTemperaturaAC);
    ac_matter.onChangeMode(mudouModoAC);
    ac_matter.begin();

    Matter.begin();
    recomissionarMatter();

    estado = ar_condicionado.getState();
    draw_current_state(&(estado));
}

void loop() {
    botao.process();
    reconectarWiFi();
    recomissionarMatter();
}