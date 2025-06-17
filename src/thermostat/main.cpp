#include <Arduino.h>
#include "ACcontrol.h"
#include "EInkPaper.h"

unsigned long antes, agora = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    setup_AC();
    setup_EInk();

    fontes.setFont(u8g2_font_helvB24_te);
    fontes.setFontMode(1);
    fontes.setCursor(0, 50);
    fontes.print("Eu odeio led IR");
    tela.display(true);
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