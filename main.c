#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

#include "shared_vars.h"
#include "wifi.h"
#include "mqtt.h"
#include "temperature.h"

// --- Constantes de Controle ---
#define TEMPERATURE_READ_INTERVAL_MS 5000
#define MQTT_PUBLISH_INTERVAL_MS 10000 // Publica a cada 10 segundos

int main() {
    stdio_init_all();

    // Inicializações de hardware
    adc_init();

    // Tenta conectar ao Wi-Fi
    if (!wifi_connect()) {
        printf("Falha crítica ao conectar no Wi-Fi. O programa será travado.\n");
        while(true); // Trava se não conseguir conectar na inicialização
    }

    printf("Inicialização completa. Entrando no loop principal...\n");

    // Timers para as tarefas não-bloqueantes
    absolute_time_t next_temp_read = get_absolute_time();
    absolute_time_t next_mqtt_publish = get_absolute_time();

    while (true) {
        // ESSENCIAL: Mantém a pilha de rede do Wi-Fi funcionando
        cyw43_arch_poll();

        // 1: Ler a temperatura periodicamente ---
        if (time_reached(next_temp_read)) {
            temperatura_atual = read_onboard_temp_celsius();
            printf("[MAIN] Temperatura lida: %.2f C\n", temperatura_atual);
            next_temp_read = make_timeout_time_ms(TEMPERATURE_READ_INTERVAL_MS);
        }

        // 2: Gerenciar a conexão MQTT ---
        // Se o Wi-Fi estiver OK, mas o MQTT não, tenta (re)conectar.
        if (g_wifi_connected && !g_mqtt_connected) {
            printf("[MAIN] Wi-Fi OK, tentando conectar ao Broker MQTT...\n");
            
            // A nova função mqtt_connect() cuida de todo o processo de conexão.
            if (mqtt_connect()) {
                // Se conectou com sucesso, força uma publicação imediata.
                next_mqtt_publish = get_absolute_time();
            } else {
                printf("[MAIN] Falha ao conectar ao MQTT. Tentando novamente em 10s...\n");
                sleep_ms(10000); // Pausa antes de uma nova tentativa completa.
            }
        }

        // 3: Publicar dados via MQTT ---
        // Só tenta publicar se a conexão MQTT estiver ativa.
        if (g_mqtt_connected && time_reached(next_mqtt_publish)) {
            char payload[16];
            snprintf(payload, sizeof(payload), "%.2f", temperatura_atual);

            // Usa a nova função de publicação, simples e direta.
            if (!mqtt_publish(MQTT_TOPICO_TEMPERATURA, payload)) {
                printf("[MAIN] Falha ao publicar. A conexão pode ter caído.\n");
                // A própria função mqtt_publish (ou a camada inferior) deve setar g_mqtt_connected = false,
                // o que fará a TAREFA 2 tentar reconectar automaticamente no próximo ciclo.
            }
            
            // Agenda a próxima publicação.
            next_mqtt_publish = make_timeout_time_ms(MQTT_PUBLISH_INTERVAL_MS);
        }

        // Uma pequena pausa não-bloqueante para evitar 100% de uso da CPU.
        // É mais eficiente que um sleep_ms() longo.
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1));
    }
}