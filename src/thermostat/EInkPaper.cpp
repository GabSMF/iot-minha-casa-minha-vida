#include "EInkPaper.h"
#include "ACcontrol.h"

GxEPD2_290_T94_V2 modeloTela(10, 14, 15, 16);
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);
U8G2_FOR_ADAFRUIT_GFX fontes;

void setup_EInk() {
    tela.init();
    tela.setRotation(3);
    tela.fillScreen(GxEPD_WHITE);
    tela.display(true);

    fontes.begin(tela);
    fontes.setForegroundColor(GxEPD_BLACK);
}

void draw_borders() {
    tela.drawLine(150, 0, 150, 128, GxEPD_BLACK);
    tela.drawLine(150, 40, 296, 40, GxEPD_BLACK);
    tela.drawLine(150, 80, 296, 80, GxEPD_BLACK);
}

void draw_fan_speed(stdAc::fanspeed_t vel) {
    if (vel == stdAc::fanspeed_t::kAuto) {
        fontes.setCursor(190, 65);
        fontes.print("FAN AUTO");
    }
    else {
        int xcoord = 190, ycoord = 70;
        for (int i = 1; i <= static_cast<int>(vel); i++) {
            tela.fillRect(xcoord, ycoord, 10, 75-ycoord, GxEPD_BLACK);
            xcoord += 15;
            ycoord -= 5;
        }
    }
}

void draw_current_state(stdAc::state_t *estado) {
    tela.fillScreen(GxEPD_WHITE);
    draw_borders();

    // Simbolos
    fontes.setFont(u8g2_font_open_iconic_all_6x_t);
    fontes.setFontMode(1);
    fontes.drawGlyph(0, 50, 0x00ee);    // (temperatura)
    fontes.setFont(u8g2_font_open_iconic_all_4x_t);
    fontes.setFontMode(1);
    fontes.drawGlyph(155, 35, 0x0081); // engrenagem (mode)
    fontes.drawGlyph(155, 75, 0x008D);    // velocimetro (ventilador)
    if (estado->power) {
        fontes.drawGlyph(155, 120, 0x0103);    // sol (ligado)
    }
    else {
        fontes.drawGlyph(155, 120, 0x00df); // lua (desligado)
    }


    // Temperatura
    fontes.setFont(u8g2_font_helvB24_te);
    fontes.setFontMode(1);
    fontes.setCursor(0, 100);
    fontes.print(String(estado->degrees, 1U) + " °C");

    // Modo
    fontes.setFont(u8g2_font_helvB12_te);
    fontes.setFontMode(1);
    fontes.setCursor(190, 25);
    switch (estado->mode) {
        using namespace stdAc;
        case opmode_t::kAuto:
            fontes.print("AUTO");
            break;
        case opmode_t::kCool:
            fontes.print("COOL");
            break;
        case opmode_t::kDry:
            fontes.print("SECAR");
            break;
        case opmode_t::kFan:
            fontes.print("VENTILAR");
            break;
        case opmode_t::kHeat:
            fontes.print("AQUECER");
            break;
        case opmode_t::kOff:
            fontes.print("DESLIGADO");
            break;
    }

    // Vel. ventilador
    draw_fan_speed(estado->fanspeed);

    // Power
    fontes.setCursor(190, 110);
    if (estado->power) {
        fontes.print("LIGADO");
    }
    else {
        fontes.print("DESLIGADO");
    }

    tela.display(true);
}