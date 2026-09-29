#ifndef SSL_CONFIG_H
#define SSL_CONFIG_H

#include <openssl/ssl.h>
#include <openssl/err.h>

// Inicializa OpenSSL y retorna un contexto SSL_CTX configurado con los certificados
SSL_CTX *init_ssl_context(const char *cert_path, const char *key_path);

// Libera los recursos de OpenSSL al apagar el servidor
void cleanup_ssl_context(SSL_CTX *ctx);

#endif