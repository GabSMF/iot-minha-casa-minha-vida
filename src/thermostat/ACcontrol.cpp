#include "ACcontrol.h"

IRac ar_condicionado(IRled);

void setup_AC() {
    ar_condicionado.next.protocol = decode_type_t::DAIKIN;  // Set a protocol to use.
    ar_condicionado.next.model = 1;  // Some A/Cs have different models. Try just the first.
    ar_condicionado.next.mode = stdAc::opmode_t::kCool;  // Run in cool mode initially.
    ar_condicionado.next.celsius = true;  // Use Celsius for temp units. False = Fahrenheit
    ar_condicionado.next.degrees = 25;  // 25 degrees.
    ar_condicionado.next.fanspeed = stdAc::fanspeed_t::kAuto;  // Start the fan at medium.
    ar_condicionado.next.swingv = stdAc::swingv_t::kOff;  // Don't swing the fan up or down.
    ar_condicionado.next.swingh = stdAc::swingh_t::kOff;  // Don't swing the fan left or right.
    ar_condicionado.next.light = false;  // Turn off any LED/Lights/Display that we can.
    ar_condicionado.next.beep = false;  // Turn off any beep from the A/C if we can.
    ar_condicionado.next.econo = false;  // Turn off any economy modes if we can.
    ar_condicionado.next.filter = false;  // Turn off any Ion/Mold/Health filters if we can.
    ar_condicionado.next.turbo = false;  // Don't use any turbo/powerful/etc modes.
    ar_condicionado.next.quiet = false;  // Don't use any quiet/silent/etc modes.
    ar_condicionado.next.sleep = -1;  // Don't set any sleep time or modes.
    ar_condicionado.next.clean = false;  // Turn off any Cleaning options if we can.
    ar_condicionado.next.clock = -1;  // Don't set any current time if we can avoid it.
    ar_condicionado.next.power = false;  // Initially start with the unit off.
}

void loop_protocolos(acCmd::Command *comando) {
    decode_type_t protocolo = COOLIX;
    if (ar_condicionado.isProtocolSupported(protocolo)) {
        stdAc::state_t estado = ar_condicionado.next;
        estado.protocol = protocolo;
        
        switch (comando->tipo) {
            case commandType::Power:
                estado.power = comando->instrucao.ligar;
                break;
            case commandType::OpMode:
                estado.mode = comando->instrucao.modo_operacao;
                break;
            case commandType::Temperature:
                estado.degrees = comando->instrucao.temperatura;
                break;
            case commandType::FanSpeed:
                estado.fanspeed = comando->instrucao.vel_ventilador;
                break;
            default:
                Serial.println("Comando invalido!");
                break;
        }

        Serial.println("Enviando comando a um ar condicionado "+typeToString(protocolo));
        ar_condicionado.next = estado;
        ar_condicionado.sendAc();
    }
}

void send_coolix(acCmd::Command *comando) {
    ar_condicionado.next.protocol = CARRIER_AC;
    ar_condicionado.next.power = true;
    ar_condicionado.sendAc();
    Serial.println("ENVIANDO COMANDO AO AR CONDICIONADO CARRIER");
}