#include "lib/server.h"

#define PORT 8080

int main(void) {
    Server server = create_server(PORT);

    start_server(&server);

    return 0;
}