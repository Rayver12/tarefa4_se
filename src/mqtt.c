#include "mqtt.h"
#include "pico_net.h"
#include "shared_vars.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/cyw43_arch.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"

// --- Variáveis Estáticas do Módulo ---
static const unsigned char psk[] = { 0xAB, 0xCD, 0x72, 0xEF, 0x12, 0x34 };
static mbedtls_ssl_context ssl;
static mbedtls_ssl_config conf;
static pico_net_context server_fd;
static mbedtls_ctr_drbg_context ctr_drbg;
static mbedtls_entropy_context entropy;

// --- Protótipos de Funções Privadas ---
static int mqtt_send_packet(const uint8_t *buf, size_t len);
static void my_debug(void *ctx, int level, const char *file, int line, const char *str);
static void mqtt_cleanup(void);

// --- Implementações ---

/**
 * @brief Envia um pacote MQTT genérico pela conexão TLS.
 */
static int mqtt_send_packet(const uint8_t *buf, size_t len) {
    int ret;
    size_t sent = 0;

    while (sent < len) {
        ret = mbedtls_ssl_write(&ssl, buf + sent, len - sent);
        if (ret > 0) {
            sent += ret;
            continue;
        }
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            cyw43_arch_poll(); // Permite que a rede processe
            continue;
        }
        printf("[MQTT] Erro em mbedtls_ssl_write: -0x%x\n", -ret);
        return -1; // Falha
    }
    return (int)sent;
}

/**
 * @brief Publica uma mensagem em um tópico MQTT.
 */
bool mqtt_publish(const char *topic, const char *payload) {
    if (!g_mqtt_connected) {
        printf("[MQTT] Não é possível publicar: desconectado.\n");
        return false;
    }

    // Limite simples para o tamanho do pacote
    uint8_t packet[256];
    size_t pos = 0;

    // Cabeçalho Fixo do Publish (QoS 0)
    packet[pos++] = 0x30;
    size_t rl_pos = pos++; // Posição para o "Remaining Length"

    // Comprimento do Tópico + Tópico
    size_t topic_len = strlen(topic);
    packet[pos++] = (topic_len >> 8) & 0xFF;
    packet[pos++] = topic_len & 0xFF;
    memcpy(&packet[pos], topic, topic_len);
    pos += topic_len;

    // Payload (mensagem)
    size_t payload_len = strlen(payload);
    memcpy(&packet[pos], payload, payload_len);
    pos += payload_len;

    // Calcula e insere o "Remaining Length"
    size_t remaining_length = pos - 2;
    packet[rl_pos] = remaining_length;

    // Envia o pacote completo
    printf("[MQTT] Publicando '%s' em '%s'\n", payload, topic);
    if (mqtt_send_packet(packet, pos) > 0) {
        return true;
    }
    
    // Se o envio falhar, assume que a conexão caiu
    g_mqtt_connected = false;
    return false;
}

/**
 * @brief Estabelece a conexão com o broker MQTT.
 */
bool mqtt_connect(void) {
    int ret;
    char error_buf[100]; // Buffer para mensagens de erro

    // 1. Inicializa todas as estruturas necessárias
    pico_net_init(&server_fd);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0)) != 0) {
        printf("[MQTT] Falha em mbedtls_ctr_drbg_seed: -0x%x\n", -ret);
        goto error;
    }

    // 2. Conecta via TCP usando a camada de rede (pico_net)
    printf("[MQTT] Conectando TCP a %s:%s...\n", BROKER_HOST, BROKER_PORT);
    if (!pico_net_connect(&server_fd, BROKER_HOST, atoi(BROKER_PORT))) {
        printf("[MQTT] Falha na conexão TCP.\n");
        goto error;
    }
    absolute_time_t timeout = make_timeout_time_ms(10000); // 10s timeout
    while (server_fd.state == CONN_CONNECTING && !time_reached(timeout)) {
        cyw43_arch_poll();
    }
    if (server_fd.state != CONN_CONNECTED) {
        printf("[MQTT] Timeout/falha na conexão TCP.\n");
        goto error;
    }

    // 3. Configura o SSL/TLS
    if ((ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        printf("[MQTT] Falha em mbedtls_ssl_config_defaults: -0x%x\n", -ret);
        goto error;
    }
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    // 4. Configura a autenticação PSK (Pre-Shared Key)
    if ((ret = mbedtls_ssl_conf_psk(&conf, psk, sizeof(psk), (const unsigned char *)PSK_IDENTITY, strlen(PSK_IDENTITY))) != 0) {
        printf("[MQTT] Falha em mbedtls_ssl_conf_psk: -0x%x\n", -ret);
        goto error;
    }
    const int ciphersuite[] = { MBEDTLS_TLS_PSK_WITH_AES_128_CBC_SHA256, 0 };
    mbedtls_ssl_conf_ciphersuites(&conf, ciphersuite);
    
    // 5. Associa a configuração SSL e os callbacks de rede
    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
        printf("[MQTT] Falha em mbedtls_ssl_setup: -0x%x\n", -ret);
        goto error;
    }
    mbedtls_ssl_set_bio(&ssl, &server_fd, (mbedtls_ssl_send_t *)pico_net_send, (mbedtls_ssl_recv_t *)pico_net_recv, NULL);

    // 6. Realiza o Handshake TLS
    printf("[MQTT] Realizando handshake TLS...\n");
    timeout = make_timeout_time_ms(10000);
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            mbedtls_strerror(ret, error_buf, sizeof(error_buf));
            printf("[MQTT] Handshake falhou: -0x%x -> %s\n", -ret, error_buf);
            goto error;
        }
        if (time_reached(timeout)) {
            printf("[MQTT] Timeout no handshake.\n");
            goto error;
        }
        cyw43_arch_poll();
    }
    printf("[MQTT] Handshake TLS bem-sucedido!\n");

    // 7. Envia o pacote MQTT CONNECT
    printf("[MQTT] Enviando pacote CONNECT...\n");
    uint8_t packet[128];
    size_t pos = 0;

    // Cabeçalho Fixo
    packet[pos++] = 0x10; // Tipo de Pacote: CONNECT
    size_t rl_pos = pos++; // Guarda a posição do Remaining Length

    // Cabeçalho Variável: Nome do Protocolo ("MQTT")
    const char *proto_name = "MQTT";
    packet[pos++] = 0x00; // Comprimento MSB
    packet[pos++] = strlen(proto_name); // Comprimento LSB
    memcpy(&packet[pos], proto_name, strlen(proto_name));
    pos += strlen(proto_name);

    // Cabeçalho Variável: Nível do Protocolo
    packet[pos++] = 4; // MQTT v3.1.1

    // Cabeçalho Variável: Flags de Conexão
    packet[pos++] = 0x02; // Apenas Clean Session = 1

    // Cabeçalho Variável: Keep Alive (60 segundos)
    packet[pos++] = 0x00; // Keep Alive MSB
    packet[pos++] = 60;   // Keep Alive LSB

    // Payload: Client ID
    size_t client_id_len = strlen(DEVICE_ID);
    packet[pos++] = (client_id_len >> 8) & 0xFF; // Comprimento MSB
    packet[pos++] = client_id_len & 0xFF;        // Comprimento LSB
    memcpy(&packet[pos], DEVICE_ID, client_id_len);
    pos += client_id_len;

    // Atualiza o campo "Remaining Length" que deixamos em branco
    packet[rl_pos] = pos - 2;

    if (mqtt_send_packet(packet, pos) <= 0) {
        printf("[MQTT] Falha ao enviar pacote CONNECT.\n");
        goto error;
    }

    // 8. Aguarda o CONNACK do broker
    printf("[MQTT] Pacote CONNECT enviado. Aguardando CONNACK...\n");
    uint8_t connack_resp[4];
    timeout = make_timeout_time_ms(5000); // Timeout de 5s para a resposta
    int bytes_read = 0;

    // Loop não-bloqueante para ler a resposta completa
    while (bytes_read < sizeof(connack_resp) && !time_reached(timeout)) {
        ret = mbedtls_ssl_read(&ssl, connack_resp + bytes_read, sizeof(connack_resp) - bytes_read);
        if (ret > 0) {
            bytes_read += ret;
        } else if (ret != MBEDTLS_ERR_SSL_WANT_READ) {
            mbedtls_strerror(ret, error_buf, sizeof(error_buf));
            printf("[MQTT] Falha ao ler CONNACK: -0x%x -> %s\n", -ret, error_buf);
            goto error;
        }
        cyw43_arch_poll(); // Permite que a rede trabalhe
    }

    if (bytes_read < sizeof(connack_resp)) {
        printf("[MQTT] Timeout esperando por CONNACK.\n");
        goto error;
    }
    
    // Agora verifica a resposta recebida
    if (connack_resp[0] == 0x20 && connack_resp[1] == 0x02 && connack_resp[3] == 0x00) {
        printf("[MQTT] Conexão MQTT estabelecida!\n");
        g_mqtt_connected = true;
        return true; // Sucesso!
    } else {
        printf("[MQTT] CONNACK inválido (código: 0x%02x). Conexão rejeitada.\n", connack_resp[3]);
        goto error;
    }

error:
    mqtt_cleanup(); // Libera todos os recursos em caso de falha
    return false;
}

/**
 * @brief Libera todos os recursos de rede e TLS.
 */
static void mqtt_cleanup(void) {
    printf("[MQTT] Limpando recursos...\n");
    mbedtls_ssl_close_notify(&ssl);
    pico_net_close(&server_fd);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    g_mqtt_connected = false;
}

// Opcional: callback de debug do mbedTLS
static void my_debug(void *ctx, int level, const char *file, int line, const char *str) {
    // Para depuração intensa, você pode habilitar isso
    printf("mbedTLS: %s:%04d: %s", file, line, str);
}