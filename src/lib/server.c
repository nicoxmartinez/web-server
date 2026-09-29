#include "server.h"
#include "sslConfig.h"
#include "sendFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

Server create_server(int port)
{
    Server server;
    server.port = port;
    server.is_running = 0;

    // Inicializar el contexto TLS mediante el módulo sslConfig
    server.ssl_ctx = init_ssl_context("certs/cert.pem", "certs/key.pem");

    // Crear el socket (domain: IPv4, type: SOCK_STREAM, protocol: TCP)
    server.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server.server_fd == 0)
    {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Permitir reutilizar el puerto/direccion inmediatamente si reiniciamos el servidor
    int opt = 1;
    if (setsockopt(server.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Error en setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    server.address.sin_family = AF_INET;
    server.address.sin_addr.s_addr = INADDR_ANY;
    server.address.sin_port = htons(port);

    // Enlazar el socket
    if (bind(server.server_fd, (struct sockaddr *)&server.address, sizeof(server.address)) < 0)
    {
        perror("Error en enlazar el socket con el puerto");
        exit(EXIT_FAILURE);
    }

    return server;
}

void start_server(Server *server)
{
    // Poner el socket en modo escucha (cola de 10 conexiones)
    if (listen(server->server_fd, 10) < 0)
    {
        perror("Error en escuchar las conexiones entrantes");
        exit(EXIT_FAILURE);
    }

    server->is_running = 1;
    printf("Servidor HTTPS corriendo en el puerto %d...\n", server->port);

    int addrlen = sizeof(server->address);
    char buffer[BUFFER_SIZE] = {0};

    // Bucle principal para aceptar clientes
    while (server->is_running)
    {
        int client_socket = accept(server->server_fd, (struct sockaddr *)&server->address, (socklen_t *)&addrlen);
        if (client_socket < 0)
        {
            perror("Error al aceptar conexión");
            continue;
        }

        // Crear instancia SSL vinculada al socket del cliente
        SSL *ssl = SSL_new(server->ssl_ctx);
        SSL_set_fd(ssl, client_socket);

        // Realizar el Handshake TLS
        int ssl_err = SSL_accept(ssl);
        if (ssl_err <= 0)
        {
            int err_code = SSL_get_error(ssl, ssl_err);
            printf("Fallo en SSL_accept (Código de error SSL: %d)\n", err_code);
            ERR_print_errors_fp(stderr);
        }
        else
        {
            // Leer la petición HTTP cifrada enviada por el cliente/navegador
            memset(buffer, 0, BUFFER_SIZE);
            SSL_read(ssl, buffer, BUFFER_SIZE - 1);

            char method[16], path[256], file_path[512];
            // Extraer método y ruta del buffer
            sscanf(buffer, "%s %s", method, path);
            printf("Peticion cifrada: %s %s\n", method, path);

            if (strcmp(path, "/") == 0)
            {
                strcpy(file_path, "src/static/index.html");
            }
            else
            {
                // envía un archivo estático según su nombre en la ruta indicada
                snprintf(file_path, sizeof(file_path), "src/static%s.html", path);
            }

            // servir el archivo correspondiente cifrado
            send_file(ssl, file_path, "200 OK");
        }

        // Liberar recursos SSL y cerrar socket
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client_socket);
    }
}

void stop_server(Server *server)
{
    if (server->is_running)
    {
        server->is_running = 0;
        
        // Cerrar el socket POSIX
        close(server->server_fd);

        // Liberar el contexto SSL y limpiar la librería OpenSSL
        cleanup_ssl_context(server->ssl_ctx);

        printf("Servidor HTTPS detenido.\n");
    }
}