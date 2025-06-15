#include "ACcontrol.h"

void loop_protocolos(acCmd::Command *comando) {
    for (int i=1; i < kLastDecodeType; i++) {
        decode_type_t protocolo = (decode_type_t) i;

        if (ar_condicionado.isProtocolSupported(protocolo)) {
            stdAc::state_t estado = ar_condicionado.getStatePrev();
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

            Serial.println("Enviando comando a um ar condicionado "+String(protocolo));
            ar_condicionado.next = estado;
            ar_condicionado.sendAc();
        }
    }
}