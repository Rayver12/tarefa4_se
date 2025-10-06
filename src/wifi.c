#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "shared_vars.h"
#include "wifi.h"
//#include "led.h"

// Inicializa a interface Wi-Fi.
void wifi_init(void) {
    if (cyw43_arch_init()) {
        printf("Falha ao inicializar a interface Wi-Fi\n");
        // Em caso de falha aqui, o sistema trava, pois é um erro de hardware.
        while(true);
    }
    cyw43_arch_enable_sta_mode();
    printf("Interface Wi-Fi inicializada.\n");
}


// Tenta conectar ao Wi-Fi. Retorna true em sucesso, false em falha.
bool wifi_connect(void) {
    printf("Conectando-se ao Wi-Fi '%s'...\n", WIFI_SSID);

    // Tenta conectar por até 30 segundos
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000) != 0) {
        printf("Falha ao conectar.\n");
        return false;
    }

    printf("Conexão WiFi bem-sucedida!\n");
    g_wifi_connected = true;
    return true;
}