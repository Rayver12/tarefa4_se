#include "shared_vars.h"

//mpu6050_data_t g_dados_sensor; // Inicializado com zeros por padrão
volatile float temperatura_atual = 0.0f;
volatile bool g_wifi_connected = false;
volatile bool g_mqtt_connected = false;
