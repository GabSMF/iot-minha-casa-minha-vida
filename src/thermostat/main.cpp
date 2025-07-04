#include <Arduino.h>
#include "ACcontrol.h"
#include "EInkPaper.h"
#include "MatterACEndpoint.h"
#include "WiFiComms.h"

unsigned long antes, agora = 0;
stdAc::state_t estado;

MatterAC ac_matter;

void setup() {
    Serial.begin(115200);
    delay(2000);

    ACpreferences.begin("ac_pref");

    setup_AC();
    setup_EInk();
    ac_matter.begin();

    Matter.begin();
    recomissionarMatter();

    estado = ar_condicionado.getState();
    draw_current_state(&(estado));
}

void loop() {
    acCmd::Command comando;
    comando.tipo = commandType::Power;
    comando.instrucao.ligar = false;

    agora = millis();
    if (agora > antes + 1500) {
        antes = agora;
        loop_protocolos(&comando);
    }

    recomissionarMatter();
}