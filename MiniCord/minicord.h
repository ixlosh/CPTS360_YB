#ifndef MINICORD_H
#define MINICORD_H

#include <stdbool.h>
#include <poll.h>

#define MAX_CLIENTS 100
#define MAX_USERNAME 32
#define MAX_CHANNEL 32

// Client state tracker
typedef struct {
    int fd;
    bool is_authenticated;
    char username[MAX_USERNAME];
    char channel[MAX_CHANNEL];
} Client;

int setup_server_socket(int port);

#endif