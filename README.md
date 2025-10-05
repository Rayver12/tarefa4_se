# Firmware MQTT com TLS-PSK para Raspberry Pi Pico W

## Descrição

Este é um projeto de firmware desenvolvido em C para a placa Raspberry Pi Pico W. O principal objetivo deste firmware é ler o sensor de temperatura interno do microcontrolador e enviar esses dados de forma segura para um broker MQTT público.

A comunicação é protegida utilizando o protocolo TLS com autenticação por Chave Pré-Compartilhada (PSK - Pre-Shared Key), garantindo que apenas dispositivos autorizados possam se conectar e publicar dados. O firmware se conecta a uma rede Wi-Fi pré-configurada e publica a temperatura em um tópico MQTT específico em intervalos regulares.

## Funcionalidades Principais

* Inicialização do hardware do Raspberry Pi Pico W, incluindo o módulo Wi-Fi.
* Leitura do sensor de temperatura interno do chip RP2040.
* Conexão a uma rede Wi-Fi utilizando credenciais pré-definidas.
* Estabelecimento de uma conexão segura (TLS-PSK) com um broker MQTT.
* Publicação periódica dos dados de temperatura em um tópico MQTT.
* Logs de status e erros enviados via comunicação serial (USB).

## Pré-requisitos

### Hardware

* **Placa:** Raspberry Pi Pico W
* **Cabo:** Micro USB para conexão com o computador

### Software

* **Pico SDK:** SDK oficial para desenvolvimento no RP2040.
* **Compilador:** `arm-none-eabi-gcc`
* **Build System:** `CMake` (versão 3.13 ou superior)
* **Cliente MQTT para Teste:** `mosquitto-clients` (para usar o `mosquitto_sub`)

## Configuração

* Todas as informações sensíveis, como credenciais de rede e chaves de autenticação, devem ser configuradas no arquivo `shared_vars.h`.

```c
// shared_vars.h - Arquivo de Configuração

// --- Configurações da Rede Wi-Fi ---
#define WIFI_SSID         "SEU_SSID_AQUI"
#define WIFI_PASSWORD     "SUA_SENHA_AQUI"

// --- Configurações do Broker MQTT ---
#define BROKER_HOST     "127.0.0.1"  // IP do Servidor
#define BROKER_PORT     "8883"
#define PSK_IDENTITY    "aluno02"
#define DEVICE_ID       "bitdoglab-aluno02-xx"  // ID do dispositivo (client ID MQTT)
#define MQTT_TOPICO_TEMPERATURA "/aluno02/bitdoglab/temperatura"
```

* Por fim, defina a chave psk no formado de array de bytes no local indicado do arquivo `mqtt.c`.

```c
// --- Exemplo de chave para: "ABCD02EF1234" ---
static const unsigned char psk[] = { 0xAB, 0xCD, 0x02, 0xEF, 0x12, 0x34 };

// --- Exemplo de chave para: "ABCD70EF1234" ---
static const unsigned char psk[] = { 0xAB, 0xCD, 0x70, 0xEF, 0x12, 0x34 };
```
