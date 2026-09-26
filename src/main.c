#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "lib/sendFile.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int main(void)
{
    int server_socket, new_socket;
    struct sockaddr_in server_address;
    int addrlen = sizeof(server_address);
    char buffer[BUFFER_SIZE] = {0};

    // Crear el socket (domain: IPv4, type: SOCK_STREAM, protocol: TCP)
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("No se pudo crear el socket");
        exit(EXIT_FAILURE);
    }

    // Limpia cualquier valor basura en la memoria antes de asignar los campos de la estructura.
    memset(&server_address, 0, sizeof(server_address));
    // Configurar la dirección del servidor
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY; // Escuchar en cualquier interfaz local
    server_address.sin_port = htons(PORT);       // Convertir puerto a Big-Endian

    // Vincular el socket al puerto
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("No se pudo conectar con el servidor");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Poner el socket en modo escucha (cola de 10 conexiones)
    if (listen(server_socket, 10) < 0)
    {
        perror("No se pudo escuchar en el socket del servidor");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Permitir reutilizar el puerto inmediatamente si reiniciamos el servidor
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Error en setsockopt");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    // Verificar porque no esta dando resultados

    printf("Servidor escuchando en el puerto:%d\n", PORT);

    // Bucle principal para aceptar clientes
    while (1)
    {
        new_socket = accept(server_socket, (struct sockaddr *)&server_address, (socklen_t *)&addrlen);
        if (new_socket < 0)
        {
            perror("Error en aceptar el cliente");
            continue;
        }

        // Leer la petición HTTP enviada por el cliente/navegador
        ssize_t bytes_read = read(new_socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';

            char method[10];
            char path[256];

            // Extraer método y ruta del buffer
            sscanf(buffer, "%s %s", method, path);
            printf("Peticion: %s %s\n", method, path);

            // Construir la ruta relativa al archivo
            char file_path[512];

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
            send_file(new_socket, file_path, "200 OK");
        }
    }
    // Cerrar el socket principal
    close(server_socket);

    return 0;
}