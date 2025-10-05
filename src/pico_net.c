#include "pico_net.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include <string.h>
#include "mbedtls/net_sockets.h"

/*
 * Protótipos das funções de callback estáticas.
 * Essas funções são usadas internamente pelo lwIP para gerenciar eventos de conexão TCP,
 * como estabelecimento de conexão, recebimento de dados e erros. [web:5]
 */

// net_connected_cb: Callback chamada quando a conexão TCP é estabelecida com sucesso ou falha.
// Ela atualiza o estado da conexão no contexto e notifica via printf se bem-sucedida. [web:5]
static err_t net_connected_cb(void *arg, struct tcp_pcb *tpcb, err_t err);

// net_recv_cb: Callback chamada quando dados são recebidos na conexão TCP.
// Ela gerencia o buffer de recepção (pbuf) concatenando os dados recebidos. [web:5]
static err_t net_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

// net_error_cb: Callback chamada em caso de erro na conexão TCP.
// Ela atualiza o estado para falha e registra o erro via printf. [web:5]
static void net_error_cb(void *arg, err_t err);

/*
 * pico_net_init: Inicializa o contexto da rede (pico_net_context).
 * Limpa a memória do contexto, define o estado inicial como ocioso (CONN_IDLE),
 * e inicializa o buffer de recepção e offset como nulos/zero. [web:5]
 */
void pico_net_init(pico_net_context *ctx) {
    memset(ctx, 0, sizeof(pico_net_context));
    ctx->state = CONN_IDLE;
    ctx->rx_buf = NULL;
    ctx->rx_offset = 0;
}

/*
 * pico_net_connect: Inicia uma conexão TCP com o host especificado pelo IP e porta.
 * Cria um novo PCB TCP usando tcp_new(), configura callbacks, converte o IP string para endereço,
 * e tenta conectar usando tcp_connect(). Retorna true se a conexão for iniciada com sucesso,
 * caso contrário, falha e fecha. [web:5]
 */
bool pico_net_connect(pico_net_context *ctx, const char *host_ip, uint16_t port) {
    ip_addr_t target_ip;
    ip4addr_aton(host_ip, &target_ip);

    ctx->pcb = tcp_new();
    if (!ctx->pcb) return false;

    tcp_arg(ctx->pcb, ctx);
    tcp_err(ctx->pcb, net_error_cb);
    tcp_recv(ctx->pcb, net_recv_cb);

    ctx->state = CONN_CONNECTING;
    err_t err = tcp_connect(ctx->pcb, &target_ip, port, net_connected_cb);
    if (err != ERR_OK) {
        ctx->state = CONN_FAILED;
        pico_net_close(ctx);
        return false;
    }
    return true;
}

/*
 * pico_net_send: Envia dados através da conexão TCP estabelecida.
 * Verifica se o estado é conectado, usa tcp_write para enfileirar os dados com cópia,
 * e tcp_output para transmitir. Retorna o número de bytes enviados ou erros mapeados para mbedTLS. [web:5]
 */
int pico_net_send(void *v_ctx, const unsigned char *buf, size_t len) {
    pico_net_context *ctx = (pico_net_context *)v_ctx;

    if (ctx->state != CONN_CONNECTED) return MBEDTLS_ERR_NET_CONN_RESET;

    err_t err = tcp_write(ctx->pcb, buf, len, TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) {
        err = tcp_output(ctx->pcb);
        if (err == ERR_OK) return (int)len;
        if (err == ERR_MEM) return MBEDTLS_ERR_SSL_WANT_WRITE;
    }
    return MBEDTLS_ERR_NET_SEND_FAILED;
}

/*
 * pico_net_recv: Recebe dados do buffer de recepção e copia para o buffer do usuário.
 * Se não há dados, retorna erro de leitura pendente ou reset. Copia do pbuf atual,
 * gerencia o offset e libera pbufs consumidos. Retorna o número de bytes copiados. [web:5][web:12]
 */
int pico_net_recv(void *v_ctx, unsigned char *buf, size_t len) {
    pico_net_context *net_ctx = (pico_net_context *)v_ctx;

    if (net_ctx->rx_buf == NULL) {
        if (net_ctx->state != CONN_CONNECTED) {
            return MBEDTLS_ERR_NET_CONN_RESET;
        }
        return MBEDTLS_ERR_SSL_WANT_READ;
    }

    struct pbuf *p = net_ctx->rx_buf;
    size_t available_in_this_pbuf = (size_t)p->len - net_ctx->rx_offset;
    size_t copy_len = available_in_this_pbuf > len ? len : available_in_this_pbuf;

    memcpy(buf, (const uint8_t*)p->payload + net_ctx->rx_offset, copy_len);

    if (copy_len == available_in_this_pbuf) {
        net_ctx->rx_buf = p->next;
        if (net_ctx->rx_buf != NULL) {
            pbuf_ref(net_ctx->rx_buf);
        }
        pbuf_free(p);
        net_ctx->rx_offset = 0;
    } else {
        net_ctx->rx_offset += copy_len;
    }
    return (int)copy_len;
}

/*
 * net_recv_cb: Gerencia a recepção de dados via callback do lwIP.
 * Se p é NULL, indica fechamento remoto e atualiza estado para fechando.
 * Caso contrário, concatena o pbuf recebido ao buffer de recepção usando pbuf_cat. [web:5][web:12]
 */
static err_t net_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    pico_net_context *ctx = (pico_net_context *)arg;

    if (p == NULL) {
        printf("[PICO_NET] Conexão fechada pelo broker.\n");
        ctx->state = CONN_CLOSING;
        return ERR_OK;
    }

    if (ctx->rx_buf == NULL) {
        ctx->rx_buf = p;
    } else {
        pbuf_cat(ctx->rx_buf, p);
    }
    return ERR_OK;
}

/*
 * pico_net_close: Fecha a conexão TCP e limpa recursos.
 * Remove callbacks do PCB, fecha o PCB com tcp_close, libera o buffer de recepção com pbuf_free,
 * e reseta o estado e offset para ocioso. [web:5][web:12]
 */
void pico_net_close(pico_net_context *ctx) {
    if (ctx->pcb) {
        tcp_arg(ctx->pcb, NULL);
        tcp_err(ctx->pcb, NULL);
        tcp_recv(ctx->pcb, NULL);
        tcp_sent(ctx->pcb, NULL);
        tcp_close(ctx->pcb);
        ctx->pcb = NULL;
    }
    if (ctx->rx_buf) {
        pbuf_free(ctx->rx_buf);
        ctx->rx_buf = NULL;
    }
    ctx->state = CONN_IDLE;
    ctx->rx_offset = 0;
}

/*
 * net_connected_cb: Callback para o resultado da conexão TCP (de tcp_connect).
 * Se err é OK, estabelece o estado conectado e remove callback de envio.
 * Caso contrário, define estado como falhado. Notifica via printf em sucesso. [web:5]
 */
static err_t net_connected_cb(void *arg, struct tcp_pcb *tpcb, err_t err) {
    pico_net_context *ctx = (pico_net_context *)arg;
    if (err == ERR_OK) {
        printf("[PICO_NET] Conexão TCP estabelecida!\n");
        ctx->state = CONN_CONNECTED;
        tcp_sent(tpcb, NULL);
    } else {
        ctx->state = CONN_FAILED;
    }
    return ERR_OK;
}

/*
 * net_error_cb: Callback para erros na conexão TCP.
 * Atualiza o estado para falhado e imprime o código de erro. [web:5]
 */
static void net_error_cb(void *arg, err_t err) {
    pico_net_context *ctx = (pico_net_context *)arg;
    ctx->state = CONN_FAILED;
    printf("[PICO_NET] Erro de rede: %d\n", err);
}
