#ifndef SEND_FILE_H
#define SEND_FILE_H

#include <openssl/ssl.h>

void send_file(SSL *ssl, const char *file_path, const char *http_status);

#endif