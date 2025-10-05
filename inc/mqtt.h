// mqtt.h
#ifndef MQTT_H
#define MQTT_H

#include <stdbool.h>

// Tenta estabelecer a conexão completa (TCP -> TLS -> MQTT) com o broker.
bool mqtt_connect(void);

// Publica uma mensagem de texto (payload) em um tópico.
bool mqtt_publish(const char *topic, const char *payload);

#endif