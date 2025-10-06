#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

#include "lwip/netif.h"

#include "shared_vars.h"
#include "wifi.h"
#include "mqtt.h"
#include "temperature.h"
#include "botoes.h"
#include "ssd1306.h"

// --- Constantes de Controle ---
#define TEMPERATURE_READ_INTERVAL_MS 5000
#define MQTT_PUBLISH_INTERVAL_MS 10000 // Publica a cada 10 segundos
#define MQTT_RECONNECT_INTERVAL_MS 10000 // Tenta reconectar a cada 10 segundos
#define DISPLAY_UPDATE_INTERVAL_MS 100 // *** ATUALIZA O ECRÃ 10 VEZES POR SEGUNDO ***

// --- Pinos ---
#define I2C_SDA_PIN 14
#define I2C_SCL_PIN 15

// --- Display ---
ssd1306_t disp;

void init_display() {
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    
    ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
    ssd1306_clear(&disp);
    ssd1306_draw_string(&disp, 0, 0, 1, "Iniciando...");
    ssd1306_show(&disp);
}


int main() {
    stdio_init_all();

    // Inicializações de hardware
    adc_init();
    buttons_init();
    init_display();
    wifi_init();

    // Tenta conectar ao Wi-Fi repetidamente
    while (!wifi_connect()) {
        printf("Falha ao conectar no Wi-Fi. Tentando novamente em 5 segundos...\n");
        ssd1306_clear(&disp);
        ssd1306_draw_string(&disp, 0, 0, 1, "Conectando WiFi...");
        ssd1306_show(&disp);
        sleep_ms(5000);
    }

    printf("Inicialização completa. Entrando no loop principal...\n");

    // --- Temporizadores para todas as tarefas não-bloqueantes ---
    absolute_time_t next_temp_read = get_absolute_time();
    absolute_time_t next_mqtt_publish = get_absolute_time();
    absolute_time_t next_display_update = get_absolute_time();
    absolute_time_t next_mqtt_connect_attempt = get_absolute_time();
    
    // Variáveis para o estado dos botões
    bool last_button_a_state = false;
    bool last_button_b_state = false;

    while (true) {
        // --- Loop Principal Não-Bloqueante ---

        // 1: Verifica botões (sempre, para máxima responsividade)
        buttons_check_and_handle(&last_button_a_state, &last_button_b_state);

        // 2: Ler a temperatura periodicamente
        if (time_reached(next_temp_read)) {
            temperatura_atual = read_onboard_temp_celsius();
            //printf("[MAIN] Temperatura lida: %.2f °C\n", temperatura_atual); // Opcional: pode poluir o log
            next_temp_read = make_timeout_time_ms(TEMPERATURE_READ_INTERVAL_MS);
        }

        // 3: Gerenciar a conexão MQTT de forma não-bloqueante
        if (g_wifi_connected && !g_mqtt_connected && time_reached(next_mqtt_connect_attempt)) {
            printf("[MAIN] Wi-Fi OK, tentando conectar ao Broker MQTT...\n");
            
            if (mqtt_connect()) {
                // Sucesso! Agenda a primeira publicação imediatamente.
                next_mqtt_publish = get_absolute_time();
            } else {
                // Falha! Agenda a próxima tentativa sem bloquear o loop.
                printf("[MAIN] Falha ao conectar ao MQTT. Tentando novamente em %d ms...\n", MQTT_RECONNECT_INTERVAL_MS);
                next_mqtt_connect_attempt = make_timeout_time_ms(MQTT_RECONNECT_INTERVAL_MS);
            }
        }

        // 4: Publicar dados via MQTT
        if (g_mqtt_connected && time_reached(next_mqtt_publish)) {
            char payload[16];
            snprintf(payload, sizeof(payload), "%.2f", temperatura_atual);

            if (!mqtt_publish(MQTT_TOPICO_TEMPERATURA, payload)) {
                printf("[MAIN] Falha ao publicar. A conexão pode ter caído.\n");
                // A reconexão será tratada automaticamente pelo passo 3.
                next_mqtt_connect_attempt = get_absolute_time(); // Tenta reconectar imediatamente.
            }
            
            next_mqtt_publish = make_timeout_time_ms(MQTT_PUBLISH_INTERVAL_MS);
        }
        
        // 5: Atualizar Display (agora de forma muito mais rápida)
        if (time_reached(next_display_update)) {
            ssd1306_clear(&disp);
            char line_buffer[32];

            snprintf(line_buffer, sizeof(line_buffer), "IP: %s", ip4addr_ntoa(netif_ip4_addr(netif_default)));
            ssd1306_draw_string(&disp, 0, 0, 1, line_buffer);

            snprintf(line_buffer, sizeof(line_buffer), "Temp: %.2f°C", temperatura_atual);
            ssd1306_draw_string(&disp, 0, 16, 1, line_buffer);
            
            snprintf(line_buffer, sizeof(line_buffer), "MQTT: %s", g_mqtt_connected ? "Conectado" : "Desconectado");
            ssd1306_draw_string(&disp, 0, 32, 1, line_buffer);

            snprintf(line_buffer, sizeof(line_buffer), "BTNS: A:%s B:%s", last_button_a_state ? "P" : "S", last_button_b_state ? "P" : "S");
            ssd1306_draw_string(&disp, 0, 48, 1, line_buffer);

            ssd1306_show(&disp);
            next_display_update = make_timeout_time_ms(DISPLAY_UPDATE_INTERVAL_MS);
        }

        // 6: Permite que a pilha de rede Wi-Fi funcione e cede o controlo
        // Esta função é otimizada para consumir muito pouca energia se não houver trabalho a fazer.
        cyw43_arch_poll();
        sleep_ms(1); // Um pequeno delay para evitar 100% de uso da CPU
    }
}