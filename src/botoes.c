#include "botoes.h"
#include "pico/stdlib.h"
#include "shared_vars.h"
#include "mqtt.h"
#include <stdio.h>

void buttons_init(void) {
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);
}

void buttons_check_and_handle(bool *last_a_state, bool *last_b_state) {
    bool current_a_state = !gpio_get(BUTTON_A_PIN); // Invertido: true se pressionado
    bool current_b_state = !gpio_get(BUTTON_B_PIN); // Invertido: true se pressionado

    // Verifica Botão A
    if (current_a_state != *last_a_state) {
        if (g_mqtt_connected) {
            if (current_a_state) {
                mqtt_publish(MQTT_TOPICO_BOTAO_A, "{\"estado\":\"pressionado\"}");
            } else {
                printf("[BOTOES] Botao A liberado!\n");
                mqtt_publish(MQTT_TOPICO_BOTAO_A, "{\"estado\":\"liberado\"}");
            }
        }
        *last_a_state = current_a_state;
    }

    // Verifica Botão B
    if (current_b_state != *last_b_state) {
        if (g_mqtt_connected) {
            if (current_b_state) {
                mqtt_publish(MQTT_TOPICO_BOTAO_B, "{\"estado\":\"pressionado\"}");
            } else {
                printf("[BOTOES] Botao B liberado!\n");
                mqtt_publish(MQTT_TOPICO_BOTAO_B, "{\"estado\":\"liberado\"}");
            }
        }
        *last_b_state = current_b_state;
    }
}