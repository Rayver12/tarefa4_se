#ifndef BOTOES_H
#define BOTOES_H

#include <stdbool.h>

// --- Pinos ---
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Inicializa os pinos dos botões como entradas com pull-up.
void buttons_init(void);

// Verifica o estado atual dos botões, compara com o estado anterior e publica
// mensagens MQTT em caso de mudança.
void buttons_check_and_handle(bool *last_a_state, bool *last_b_state);

#endif