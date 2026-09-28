#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>

// Estructura del servidor
typedef struct {
    int port;
    int server_fd;
    struct sockaddr_in address;
    int is_running;
} Server;

Server create_server(int port);
void start_server(Server *server);
void stop_server(Server *server);

#endif