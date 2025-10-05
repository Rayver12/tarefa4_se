#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

// ===== Núcleo TLS necessário =====
#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_PROTO_TLS1_2     // TLS 1.2 (mais seguro/estável)
#define MBEDTLS_SSL_USE_PSK          // Suporte a PSK
#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED  // NECESSÁRIO para mbedtls_ssl_conf_psk()
#define MBEDTLS_ERROR_C             // Habilita a função mbedtls_strerror()
#define MBEDTLS_DEBUG_C 

// ===== Algoritmos básicos =====
#define MBEDTLS_CIPHER_AES_ENABLED   // AES é necessário para PSK
#define MBEDTLS_AES_C                // AES é necessário para PSK
#define MBEDTLS_SHA1_C               // Necessário para TLS
#define MBEDTLS_SHA256_C             // Hash mínimo necessário
#define MBEDTLS_CIPHER_C
#define MBEDTLS_MD_C
#define MBEDTLS_CIPHER_MODE_CBC     // Necessário para TLS
#define MBEDTLS_GCM_C                // Necessário para TLS

// ===== Entropia e RNG =====
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_HARDWARE_ALT   // Usa RNG via rosc_hw
#define MBEDTLS_NO_PLATFORM_ENTROPY    // Sem SO

// ===== Plataforma =====
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY

// ===== O que NÃO precisamos =====
// Desabilitar suporte a certificados/X.509
#undef MBEDTLS_X509_USE_C
#undef MBEDTLS_X509_CRT_PARSE_C
#undef MBEDTLS_PEM_PARSE_C
#undef MBEDTLS_PK_PARSE_C

// Desabilitar recursos pesados
#undef MBEDTLS_ECP_C
#undef MBEDTLS_ECDH_C
#undef MBEDTLS_ECDSA_C
#undef MBEDTLS_BIGNUM_C
#undef MBEDTLS_ASN1_PARSE_C
#undef MBEDTLS_ASN1_WRITE_C
#undef MBEDTLS_KEY_EXCHANGE_ECDH_ECDSA_ENABLED
#undef MBEDTLS_KEY_EXCHANGE_ECDH_RSA_ENABLED
#undef MBEDTLS_KEY_EXCHANGE_ECDHE_PSK_ENABLED
#undef MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#undef MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#undef MBEDTLS_CCM_C
//#undef MBEDTLS_GCM_C

// Sem filesystem / tempo
#undef MBEDTLS_FS_IO
#undef MBEDTLS_HAVE_TIME_DATE
#undef MBEDTLS_TIMING_C

#endif /* MBEDTLS_CONFIG_H */
