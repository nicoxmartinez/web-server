#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L

#include "signalsConfig.h"
#include <stdio.h>
#include <signal.h>
#include <stddef.h>


// Referencia privada al servidor para el manejador de señales
static Server *g_server = NULL;

// Función interna invocada al matar el proceso (SIGTERM)
static void handle_shutdown_signal(int sig) {
    (void)sig; // Evita advertencias de compilador por parámetro no usado
    printf("\n[INFO] Señal de interrupción recibida. Iniciando apagado seguro...\n");
    
    if (g_server != NULL) {
        stop_server(g_server);
    }
}

void setup_signal_handlers(Server *server) {
    g_server = server;

    struct sigaction sa_ignore;
    struct sigaction sa_shutdown;

    // Ignorar SIGPIPE (Evita cierres al escribir en sockets cerrados por el cliente)
    sa_ignore.sa_handler = SIG_IGN;
    sigemptyset(&sa_ignore.sa_mask);
    sa_ignore.sa_flags = 0;
    sigaction(SIGPIPE, &sa_ignore, NULL);

    // Capturar SIGINT y SIGTERM para apagar limpiamente
    sa_shutdown.sa_handler = handle_shutdown_signal;
    sigemptyset(&sa_shutdown.sa_mask);
    sa_shutdown.sa_flags = 0;
    sigaction(SIGINT, &sa_shutdown, NULL);
    sigaction(SIGTERM, &sa_shutdown, NULL);
}