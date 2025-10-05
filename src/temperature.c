
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "temperature.h"

// Lê a temperatura interna do RP2040 em graus Celsius
float read_onboard_temp_celsius(void) {
    // Habilita o sensor de temperatura interno
    adc_set_temp_sensor_enabled(true);

    // Seleciona o canal 4 (sensor interno)
    adc_select_input(4);

    // Lê o valor bruto do ADC (12 bits)
    uint16_t raw = adc_read();

    // Converte o valor para tensão (3.3V / 4096)
    float voltage = raw * 3.3f / 4096.0f;

    // Converte a tensão em temperatura (fórmula do datasheet)
    float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;

    return temperature;
}