#include "sslConfig.h"
#include <stdio.h>
#include <stdlib.h>

static void init_openssl(void) {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

static SSL_CTX *create_ssl_context(void) {
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("No se pudo crear el contexto SSL");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

static void configure_ssl_context(SSL_CTX *ctx, const char *cert_path, const char *key_path) {
    if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

SSL_CTX *init_ssl_context(const char *cert_path, const char *key_path) {
    init_openssl();
    SSL_CTX *ctx = create_ssl_context();
    configure_ssl_context(ctx, cert_path, key_path);
    return ctx;
}

void cleanup_ssl_context(SSL_CTX *ctx) {
    if (ctx) {
        SSL_CTX_free(ctx);
    }
    EVP_cleanup();
}