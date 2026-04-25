#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "minicord.h"

// Non-blocking listener socket setup
int setup_server_socket(int port) {
    int listen_fd;
    struct sockaddr_in server_addr;

    // Create the socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Allows rebinding to the same port immediately after the server is stopped and restarted (mostly useful during development phase)
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Making the socket non-blocking so that accept() doesn't block the entire server loop
    int flags = fcntl(listen_fd, F_GETFL, 0);
    fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK);

    // Bind
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(listen_fd);
        exit(1);
    }

    // Start the listener (10 limit for overwhelm protection)
    if (listen(listen_fd, 10) < 0) {
        perror("Listen failed");
        close(listen_fd);
        exit(1);
    }

    printf("MiniCord Server initialized and listening on port %d\n", port);
    return listen_fd;
}