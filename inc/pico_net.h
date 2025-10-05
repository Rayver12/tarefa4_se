#ifndef PICO_NET_H
#define PICO_NET_H

#include <stdbool.h>
#include <mbedtls/ssl.h>

// Enum para o estado da nossa conexão
typedef enum {
    CONN_IDLE,
    CONN_CONNECTING,
    CONN_CONNECTED,
    CONN_CLOSING,
    CONN_FAILED
} conn_state_t;

// Estrutura principal que guarda o estado da rede
typedef struct {
    mbedtls_ssl_context ssl;
    struct tcp_pcb *pcb;
    volatile conn_state_t state;
    struct pbuf *rx_buf;
    size_t rx_offset; /* offset dentro do primeiro pbuf (não mexer em p->payload) */
} pico_net_context;

void pico_net_init(pico_net_context *ctx);
bool pico_net_connect(pico_net_context *ctx, const char *host_ip, uint16_t port);
void pico_net_close(pico_net_context *ctx);

// Funções de BIO para o mbedTLS (send/recv)
int pico_net_send(void *ctx, const unsigned char *buf, size_t len);
int pico_net_recv(void *ctx, unsigned char *buf, size_t len);

#endif // PICO_NET_H