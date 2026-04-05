#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "framebuffer.h"
#include "protocol.h"
#include "../include/xd.h"

static int server_fd = -1;

static void cleanup(int sig) {
    (void)sig;
    if (server_fd >= 0)
        close(server_fd);
    unlink(XD_SOCKET_PATH);
    printf("\n[server] shut down\n");
    exit(0);
}

int main(void) {
    struct sockaddr_un addr;
    int client_fd;

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    /* Remove stale socket file if it exists */
    unlink(XD_SOCKET_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, XD_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 8) < 0) {
        perror("listen");
        close(server_fd);
        unlink(XD_SOCKET_PATH);
        return 1;
    }

    fb_init();
    printf("[server] listening on %s\n", XD_SOCKET_PATH);

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        /* Handle client sequentially (single-threaded for simplicity) */
        protocol_handle_client(client_fd);
    }

    return 0;
}
