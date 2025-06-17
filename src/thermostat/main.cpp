#include <Arduino.h>
#include "ACcontrol.h"
#include "EInkPaper.h"

unsigned long antes, agora = 0;
stdAc::state_t estado;

void setup() {
    Serial.begin(115200);
    delay(2000);

    setup_AC();
    setup_EInk();

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
}