#include "server.h"
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
    printf("Servidor HTTP corriendo en el puerto %d...\n", server->port);

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

        // Leer la petición HTTP enviada por el cliente/navegador
        memset(buffer, 0, BUFFER_SIZE);
        read(client_socket, buffer, BUFFER_SIZE - 1);

        char method[16], path[256], file_path[512];
        // Extraer método y ruta del buffer
        sscanf(buffer, "%s %s", method, path);
        printf("Peticion: %s %s\n", method, path);

        if (strcmp(path, "/") == 0)
        {
            strcpy(file_path, "src/static/index.html");
        }
        else
        {
            // envia un archivo estatico segun su nombre en la ruta indicada
            snprintf(file_path, sizeof(file_path), "src/static/%s.html", path);
        }

        // servir el archivo correspondiente
        send_file(client_socket, file_path, "200 OK");

        close(client_socket);
    }
}

void stop_server(Server *server)
{
    if (server->is_running)
    {
        server->is_running = 0;
        close(server->server_fd);
        printf("Servidor detenido.\n");
    }
}