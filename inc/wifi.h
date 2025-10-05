#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>
//#include "lwip/ip_addr.h"

//Conecta o módulo Wi-Fi da placa ao ponto de acesso especificado.
bool wifi_connect(void);
extern volatile bool g_wifi_connected;

#endif
