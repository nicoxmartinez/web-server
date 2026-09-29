#include <signal.h>
#include "lib/server.h"

#define PORT 8080

int main(void) {
    signal(SIGPIPE, SIG_IGN);
    Server server = create_server(PORT);

    start_server(&server);

    return 0;
}