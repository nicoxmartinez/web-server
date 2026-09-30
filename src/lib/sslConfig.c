#include "sslConfig.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>

SSL_CTX *init_ssl_context(const char *cert_path, const char *key_path) {
    // Crear el método de servidor TLS
    const SSL_METHOD *method = TLS_server_method();
    if (method == NULL) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    // Crear el contexto SSL
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (ctx == NULL) {
        fprintf(stderr, "[ERROR] No se pudo crear el contexto SSL_CTX.\n");
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    // Cargar el certificado
    if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "[ERROR] Error al cargar el certificado PEM: %s\n", cert_path);
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }

    // Cargar la clave privada
    if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "[ERROR] Error al cargar la clave privada PEM: %s\n", key_path);
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }

    // Verificar que la clave privada coincide con el certificado
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "[ERROR] La clave privada no coincide con el certificado público.\n");
        SSL_CTX_free(ctx);
        return NULL;
    }

    return ctx;
}

void cleanup_ssl_context(SSL_CTX *ctx) {
    if (ctx != NULL) {
        SSL_CTX_free(ctx);
    }
}