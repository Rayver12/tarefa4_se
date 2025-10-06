# Firmware MQTT com TLS-PSK para Raspberry Pi Pico W

## Descrição

Este é um projeto de firmware desenvolvido em C para a placa Raspberry Pi Pico W. O objetivo principal deste firmware é ler o sensor de temperatura interno do microcontrolador e os eventos de botões, e enviar esses dados de forma segura para um broker MQTT.

A comunicação é protegida utilizando o protocolo TLS com autenticação por Chave Pré-Compartilhada (PSK - Pre-Shared Key), garantindo que apenas dispositivos autorizados possam se conectar e publicar dados. O firmware se conecta a uma rede Wi-Fi pré-configurada, exibe informações de status em um display OLED e publica os dados em tópicos MQTT específicos.

## Funcionalidades Principais

* Inicialização do hardware do Raspberry Pi Pico W, incluindo o módulo Wi-Fi.
* Leitura do sensor de temperatura interno do chip RP2040.
* Leitura de dois botões (A e B) para envio de eventos.
* Suporte a display OLED (SSD1306) para visualização de status em tempo real (IP, temperatura, status MQTT e botões).
* Conexão a uma rede Wi-Fi utilizando credenciais pré-definidas.
* Estabelecimento de uma conexão segura (TLS-PSK) com um broker MQTT.
* Publicação periódica dos dados de temperatura em um tópico MQTT.
* Publicação de eventos dos botões (pressionado/liberado) em tópicos dedicados, com payload em formato JSON.
* Arquitetura não-bloqueante no loop principal para garantir alta responsividade do display e dos botões.
* Logs de status e erros enviados via comunicação serial (USB).

## Pré-requisitos

### Hardware

* **Placa:** Raspberry Pi Pico W
* **Display:** OLED SSD1306 I2C
* **Botões:** 2x botões de pressão
* **Cabo:** Micro USB para conexão com o computador

### Software

* **Pico SDK:** SDK oficial para desenvolvimento no RP2040.
* **Compilador:** `arm-none-eabi-gcc`
* **Build System:** `CMake` (versão 3.13 ou superior)
* **Cliente MQTT para Teste:** `mosquitto-clients` (para usar o `mosquitto_sub`)

## Configuração

* Todas as informações sensíveis, como credenciais de rede e chaves de autenticação, devem ser configuradas no arquivo `shared_vars.h`.

```c
#define WIFI_SSID         "SEU_SSID_AQUI"
#define WIFI_PASSWORD     "SUA_SENHA_AQUI"

// --- Configurações do Broker MQTT ---
#define BROKER_HOST     "192.168.1.107" // IP do Servidor
#define BROKER_PORT     "8872"
#define PSK_IDENTITY    "aluno72"
#define DEVICE_ID       "bitdoglab-aluno72-xx"  // ID do dispositivo (client ID MQTT)
#define MQTT_TOPICO_TEMPERATURA "/aluno72/bitdoglab/temperatura"
#define MQTT_TOPICO_BOTAO_A     "/aluno72/bitdoglab/botoes/a"
#define MQTT_TOPICO_BOTAO_B     "/aluno72/bitdoglab/botoes/b"
```

* Por fim, defina a chave psk no formado de array de bytes no local indicado do arquivo `mqtt.c`.

```c
// --- Exemplo de chave para: "ABCD72EF1234" ---
static const unsigned char psk[] = { 0xAB, 0xCD, 0x72, 0xEF, 0x12, 0x34 };
```
