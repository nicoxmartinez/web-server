#include "./sendFile.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void send_file(SSL *ssl, const char *file_path, const char *http_status)
{
    FILE *file = fopen(file_path, "rb");

    // Si el archivo solicitado no existe envia un 404
    if (file == NULL)
    {
        perror("Error al abrir el archivo solicitado");
        if (strcmp(file_path, "src/static/notFound.html") != 0)
        {
            send_file(ssl, "src/static/notFound.html", "404 Not Found");
        }
        return;
    }

    // Obtener tamaño del archivo
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Construir cabeceras con el estado HTTP dinámico
    char headers[512];
    snprintf(headers, sizeof(headers),
             "HTTP/1.1 %s\r\n"
             "Content-Type: text/html; charset=UTF-8\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             http_status,
             file_size);

    // Enviar cabeceras
    SSL_write(ssl, headers, strlen(headers));

    // Enviar el contenido del archivo HTML
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        SSL_write(ssl, buffer, bytes_read);
    }

    fclose(file);
}