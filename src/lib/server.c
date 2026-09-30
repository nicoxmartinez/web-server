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
    memset(&server, 0, sizeof(Server));
    server.port = port;
    server.server_fd = -1;
    server.is_running = 0;

    // Inicializar el contexto TLS
    server.ssl_ctx = init_ssl_context("certs/cert.pem", "certs/key.pem");
    if (server.ssl_ctx == NULL)
    {
        fprintf(stderr, "[ERROR] Fallo al cargar el contexto SSL.\n");
        return server;
    }

    // Crear el socket (domain: IPv4, type: SOCK_STREAM, protocol: TCP)
    server.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server.server_fd < 0)
    {
        perror("[ERROR] Error al crear el socket");
        cleanup_ssl_context(server.ssl_ctx);
        server.ssl_ctx = NULL;
        return server;
    }

    // Permitir reutilizar el puerto/dirección inmediatamente si reiniciamos el servidor
    int opt = 1;
    if (setsockopt(server.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("[ERROR] Error en setsockopt");
        close(server.server_fd);
        cleanup_ssl_context(server.ssl_ctx);
        server.server_fd = -1;
        server.ssl_ctx = NULL;
        return server;
    }

    // Configurar la dirección del servidor
    server.address.sin_family = AF_INET;
    server.address.sin_addr.s_addr = INADDR_ANY;
    server.address.sin_port = htons(port);

    // Enlazar el socket con el puerto
    if (bind(server.server_fd, (struct sockaddr *)&server.address, sizeof(server.address)) < 0)
    {
        perror("[ERROR] Error al enlazar el socket con el puerto");
        close(server.server_fd);
        cleanup_ssl_context(server.ssl_ctx);
        server.server_fd = -1;
        server.ssl_ctx = NULL;
        return server;
    }

    return server;
}

void start_server(Server *server)
{
    if (server == NULL || server->ssl_ctx == NULL || server->server_fd < 0)
    {
        fprintf(stderr, "[ERROR] Intento de iniciar un servidor no configurado.\n");
        return;
    }

    // Poner el socket en modo escucha (cola de 10 conexiones)
    if (listen(server->server_fd, 10) < 0)
    {
        perror("Error en escuchar las conexiones entrantes");
        stop_server(server);
        return;
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
            // Si el socket fue cerrado por stop_server desde otra rutina/señal, salir del bucle limpiamente
            if (!server->is_running)
            {
                break;
            }
            perror("Error al aceptar conexión con el socket");
            // Seguir escuchando
            continue;
        }

        // Crear instancia SSL vinculada al socket del cliente
        SSL *ssl = SSL_new(server->ssl_ctx);
        if (ssl == NULL)
        {
            // Libero en memoria el socket
            close(client_socket);
            // seguir escuchando
            continue;
        }
        SSL_set_fd(ssl, client_socket);

        // Realizar el Handshake TLS
        int ssl_res = SSL_accept(ssl);
        if (ssl_res <= 0)
        {
            int err_code = SSL_get_error(ssl, ssl_res);

            if (err_code == SSL_ERROR_SYSCALL || err_code == SSL_ERROR_SSL)
            {
                unsigned long last_err = ERR_peek_last_error();
                int reason = ERR_GET_REASON(last_err);

                // Intento de conexión HTTP no cifrada
                if (reason == SSL_R_HTTP_REQUEST)
                {
                    fprintf(stderr, "[INFO] Intento de conexión HTTP no cifrada detectado.\n");
                }
                // El cliente (navegador) rechazó el certificado autofirmado
                else if (reason == SSL_R_SSLV3_ALERT_CERTIFICATE_UNKNOWN || reason == SSL_R_TLSV1_ALERT_UNKNOWN_CA)
                {
                    fprintf(stderr, "[INFO] El cliente desconfía del certificado autofirmado.\n");
                }
                // Cierre abrupto de socket o timeout por parte del cliente
                else if (err_code == SSL_ERROR_SYSCALL)
                {
                    fprintf(stderr, "[INFO] Conexión interrumpida por el cliente o timeout.\n");
                }
                // Errores críticos no previstos de SSL
                else
                {
                    fprintf(stderr, "[WARN] Fallo en SSL_accept (Código SSL: %d, Razon: %d)\n", err_code, reason);
                    ERR_print_errors_fp(stderr);
                }
            }

            // Limpieza estricta de la cola de errores de OpenSSL
            ERR_clear_error();

            // Cierre ordenado y liberación de recursos
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_socket);

            continue;
        }

        // Limpiar buffer y leer petición cifrada
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = SSL_read(ssl, buffer, BUFFER_SIZE - 1);

        if (bytes_read > 0)
        {
            char method[16] = {0};
            char path[256] = {0};
            char file_path[512] = {0};

            if (sscanf(buffer, "%15s %255s", method, path) == 2)
            {
                printf("Petición cifrada: %s %s\n", method, path);

                // Prevención básica de Path Traversal (evitar navegación por directorios)
                if (strstr(path, "..") != NULL)
                {
                    send_file(ssl, "src/static/400.html", "400 Bad Request");
                }
                else if (strcmp(path, "/") == 0)
                {
                    snprintf(file_path, sizeof(file_path), "src/static/index.html");
                    send_file(ssl, file_path, "200 OK");
                }
                else
                {
                    snprintf(file_path, sizeof(file_path), "src/static%s.html", path);
                    send_file(ssl, file_path, "200 OK");
                }
            }
        }

        // Cierre y liberación garantizada de la conexión TLS y el socket TCP
        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client_socket);
    }

    // Garantizar que la liberación de recursos globales se ejecute
    stop_server(server);
}

void stop_server(Server *server)
{
    if (server != NULL && server->is_running)
    {
        server->is_running = 0;

        // Cerrar el socket POSIX del servidor
        if (server->server_fd >= 0)
        {
            close(server->server_fd);
            server->server_fd = -1;
        }

        // Liberar el contexto SSL y limpiar la librería OpenSSL mediante el módulo sslConfig
        if (server->ssl_ctx != NULL)
        {
            cleanup_ssl_context(server->ssl_ctx);
            server->ssl_ctx = NULL;
        }

        printf("Servidor HTTPS detenido y recursos liberados correctamente.\n");
    }
}