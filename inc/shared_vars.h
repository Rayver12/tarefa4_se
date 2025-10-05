#ifndef SHARED_VARS_H
#define SHARED_VARS_H

#include <stdbool.h>

// =============================================================================
// Constantes de Configuração do Projeto
// =============================================================================

// --- Configurações do WiFi ---
#define WIFI_SSID       "SEU_SSID_AQUI"
#define WIFI_PASSWORD   "SUA_SENHA_AQUI"

// --- Configurações do Broker MQTT ---
#define BROKER_HOST     "127.0.0.1"  // IP do Servidor
#define BROKER_PORT     "8883"
#define PSK_IDENTITY    "aluno02"
#define DEVICE_ID       "bitdoglab-aluno02-xx"  // ID do dispositivo (client ID MQTT)
#define MQTT_TOPICO_TEMPERATURA "/aluno02/bitdoglab/temperatura"

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