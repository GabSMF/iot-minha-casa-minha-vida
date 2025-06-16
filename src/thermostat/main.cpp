#include <Arduino.h>
#include "ACcontrol.h"

unsigned long antes, agora = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    setup_AC();
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