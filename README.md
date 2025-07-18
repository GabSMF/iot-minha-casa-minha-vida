# 🏠 ﻿Projeto Smart Home IoT 

> Projeto desenvolvido como parte da disciplina Internet das Coisas (IoT) da PUC-Rio, com foco em automação e monitoramento residencial por meio de sensores conectados, dashboards e controle remoto via voz ou interface web.

<img width="3928" height="1159" alt="ideia_geral" src="https://github.com/user-attachments/assets/0f1fda1f-669c-466b-9395-5b31741ebbca" />

## 📌 Visão Geral
Este projeto implementa uma solução de casa inteligente (smart home) usando sensores, atuadores, e dashboards em tempo real. Com ele, é possível:
- Monitorar temperatura, umidade e pressão ambiente.
- Visualizar dados históricos em tempo real via Grafana.
- Controlar dispositivos como lâmpadas, ventiladores ou ar-condicionado remotamente.
- Interagir com o sistema por comandos de voz (via Alexa).
- Integrar autenticação segura com Auth0.
- Utilizar protocolos MQTT para comunicação leve entre dispositivos.

> Assista o vídeo do nosso projeto!
> https://www.youtube.com/watch?v=0ojDJ1VMN5k

# 🧰 Tecnologias e Ferramentas
| Componente                  | Descrição                                                    |
| --------------------------- | ------------------------------------------------------------ |
| **ESP32** | Dispositivo central de controle e envio de dados             |
| **MQTT**        | Protocolo de mensagens assíncronas entre sensores e servidor |
| **Grafana + PostgreSQL**    | Dashboard de visualização com base de dados                  |
| **Auth0**                   | Gerenciamento de autenticação OAuth2                         |
| **Alexa + Matter**          | Integração com assistente de voz                             |
| **Sensores, LED e Infravermelho** | Componentes principais de nossos circuitos de teste |

# ⚙️ Estrutura do Projeto
O projeto foi dividido em branches, em que cada branch representa um módulo, esses sendo:
| Branch                  | Módulo                                                    |
| --------------------------- | ------------------------------------------------------------ |
| **thermostat** | branch utilizada para desenvolver o software para controle do ar-condicionado             |
| **auth0_flask**        | branch inicial utilizada para desenvolver o sistema de login com Auth0 integrado com flask |
| **flask-server**    | branch inicialmente utilizada para desenvolvimento do servidor de preferências, mas finalizada como branch final de todo o servidor                  |
| **master**                   | branch utilizada para configuração Matter, preferências locais e configuração dos circuitos de Sensores, LED e botões                         |

# 🎨 Grafana
O Grafana foi utilizado de forma local, sem utilizar seus recursos na nuvem para melhor personalização do serviço de autenticação e exibição de dashboards específicos do usuário para isso é necessário alterar o arquivo `custom.ini` que para Windows fica no diretório `C:\Program Files\GrafanaLabs\grafana\conf\`. 

Algumas das personalizações feitas foram:

```bash
[auth.generic_oauth]
enabled = true
name = Auth0
allow_sign_up = true
client_id = YOUR_CLIENT_ID
client_secret = YOUR_CLIENT_SECRET
scopes = openid profile email
auth_url = https://YOUR_DOMAIN/authorize
token_url = https://YOUR_DOMAIN/oauth/token
api_url = https://YOUR_DOMAIN/userinfo
auto_login       = true
disable_login_form = true
allow_sign_up = true
auto_assign_org = true
auto_assign_org_role = Admin ou Viewer
```
Todas esses campos alterados permitem conectar com o Auth0 e desativar o login nativo do Grafana para que seja apenas necessária a autenticação do Auth0 feita no flask

# 🚀 Como o Projeto é executado
Para rodar o projeto é necessário executar os 3 circuitos (termostato, LED e sensores), servidor flask e servidor grafana de forma paralela. É necessário configurar um servidor no site do Auth0 o qual ficará responsável por tratar as entradas e registros de usuários, esse servidor exibe tokens que são necessários para serem inseridos para login no Grafana e no servidor flask. Lembre-se de configurar as variáveis ambiente adequadas.

# Imagens do Projeto
<img width="1177" height="546" alt="image" src="https://github.com/user-attachments/assets/28d1a525-10db-463a-9d2a-1eda565b994d" />

<img width="1161" height="547" alt="image" src="https://github.com/user-attachments/assets/c8dde081-1fa3-46ef-8d5d-66df72b3a2de" />

<img width="1107" height="576" alt="image" src="https://github.com/user-attachments/assets/15ef116c-a4c9-40cf-b5e8-fdf82fb29a52" />

<img width="752" height="437" alt="image" src="https://github.com/user-attachments/assets/c639e260-2243-4da5-a604-1775d23efe76" />

<img width="1076" height="548" alt="image" src="https://github.com/user-attachments/assets/debeb333-7d42-431e-8d38-d06a85ba4d47" />





