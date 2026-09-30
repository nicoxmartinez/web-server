#include <signal.h>
#include "lib/server.h"
#include "lib/signalsConfig.h"

#define PORT 8080

int main(void)
{
    Server server = create_server(PORT);
    if (server.server_fd < 0 || server.ssl_ctx == NULL)
    {
        fprintf(stderr, "[FATAL] No se pudo inicializar el servidor. Abortando.\n");
        return EXIT_FAILURE;
    }

    setup_signal_handlers(&server);
    start_server(&server);

    return 0;
}