#ifndef SHARED_VARS_H
#define SHARED_VARS_H

#include <stdbool.h>

// =============================================================================
// Constantes de Configuração do Projeto
// =============================================================================

// --- Configurações do WiFi ---
#define WIFI_SSID       "ED-LINK FIBRA // Joao"
#define WIFI_PASSWORD   "JOAO2FILHO8"

// --- Configurações do Broker MQTT ---
#define BROKER_HOST     "192.168.1.107"  // IP do Servidor
#define BROKER_PORT     "8872"
#define PSK_IDENTITY    "aluno72"
#define DEVICE_ID       "bitdoglab01-aluno72"  // ID do dispositivo (client ID MQTT)
#define MQTT_TOPICO_TEMPERATURA "/aluno72/bitdoglab/temp"
#define MQTT_TOPICO_BOTAO_A     "/aluno72/bitdoglab/botoes/a"
#define MQTT_TOPICO_BOTAO_B     "/aluno72/bitdoglab/botoes/b"

// =============================================================================
// Variáveis Globais Compartilhadas
// =============================================================================

// Estrutura única para armazenar os dados do sensor lidos pela task do MPU
// e consumidos pelas tasks de display e MQTT.
extern volatile float temperatura_atual;

// Flags de status de conexão
extern volatile bool g_wifi_connected;
extern volatile bool g_mqtt_connected;

#endif