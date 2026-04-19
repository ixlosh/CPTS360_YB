#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLINE 8192

void handle_client(int client_fd);

int main(int argc, char *argv[])
{
    int listen_fd, port;
    struct sockaddr_in server_addr;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(listen_fd, 10);

    printf("Proxy listening on port %d\n", port);

    while (1) {
        int client_fd = accept(listen_fd, NULL, NULL);
        handle_client(client_fd);
        close(client_fd);
    }
}

void handle_client(int client_fd)
{
    char buffer[MAXLINE];
    char method[16], uri[256], version[16];
    char hostname[256], path[256];
    int server_fd;
    struct hostent *server;

    /* ~TODO~ (DONE): Read request from client */
    ssize_t bytes_read = read(client_fd, buffer, MAXLINE - 1);
    if (bytes_read <= 0) {
        return; // No bytes could be read (there might be no bytes sent)
    }
    buffer[bytes_read] = '\0'; // Null termination

    /* ~TODO~ (DONE): Parse HTTP request line */
    sscanf(buffer, "%s %s %s", method, uri, version);

    // Only need GET
    if (strcasecmp(method, "GET") != 0) {
        return; 
    }

    /* ~TODO~ (DONE): Extract hostname and path from URI */
    // strstr will find the first occurence of "http://" and point there
    // good method to not deal with -if(!strncmp(uri, "http://", 7)) stuff
    char *host_start = strstr(uri, "http://");
    if (host_start != NULL) {
        host_start += 7; // Shift pointer forward to skip "http://"
    } else {
        host_start = uri; // no "http://" found
    }

    // Find where the hostname ends and the path begins
    char *path_start = strchr(host_start, '/');
    if (path_start != NULL) {
        strcpy(path, path_start); // Copy the path
        int host_len = path_start - host_start;
        strncpy(hostname, host_start, host_len); // Copy the hostname
        hostname[host_len] = '\0'; // Null termination
    } else {
        // If there is no trailing slash, the path is just the root directory
        // Good practice to cover all (even if not exclusively stated in assignment)
        strcpy(hostname, host_start);
        strcpy(path, "/"); 
    }

    /* ~TODO~ (DONE): Connect to remote server */
    // Create a new socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return;
    }

    // IP address conversion
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Error: No such host %s\n", hostname);
        close(server_fd);
        return;
    }

    // Destination address setup
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    // Network byte order conversion for port number (essential)
    dest_addr.sin_port = htons(80); 
    memcpy((char *)&dest_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);

    // Connect our server_fd socket to the destination web server
    if (connect(server_fd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("Connection to remote server failed");
        close(server_fd);
        return;
    }

    /* ~TODO~ (DONE): Forward request to server */
    char forward_request[MAXLINE];
    // HTTP/1.0 request format
    snprintf(forward_request, sizeof(forward_request), 
             "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", 
             path, hostname);
    
    // Write the bytes of the formatted request to the server
    write(server_fd, forward_request, strlen(forward_request));

    /* ~TODO~ (DONE): Relay response back to client */
    ssize_t n;
    while ((n = read(server_fd, buffer, MAXLINE)) > 0) {
        write(client_fd, buffer, n);
    }

    /* ~TODO~ (DONE): Close server socket */
    close(server_fd);
}