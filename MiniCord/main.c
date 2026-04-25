#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "minicord.h"

// The implementation here is based on poll_echo_server.c from the lecture materials,
// and has been significantly expanded to include authentication, channels, and direct messaging features.

// Using the same port, 9091
#define PORT 9091 

int main() {
    // Master socket initialization
    int listen_fd = setup_server_socket(PORT);

    // Arrays to track file descirptors and client states
    struct pollfd fds[MAX_CLIENTS + 1];
    Client clients[MAX_CLIENTS + 1];
    int nfds = 1;

    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;

    printf("Starting MiniCord event loop...\n");

    while (1) {
        int poll_count = poll(fds, nfds, -1);
        if (poll_count < 0) {
            perror("poll error");
            break;
        }

        // New client connection
        if (fds[0].revents & POLLIN) {
            int new_fd = accept(listen_fd, NULL, NULL);
            if (new_fd >= 0) {
                // Non-blocking
                int flags = fcntl(new_fd, F_GETFL, 0);
                fcntl(new_fd, F_SETFL, flags | O_NONBLOCK);

                // Track the new cliemt
                if (nfds <= MAX_CLIENTS) {
                    fds[nfds].fd = new_fd;
                    fds[nfds].events = POLLIN;
                    fds[nfds].revents = 0;
                    
                    clients[nfds].fd = new_fd;
                    clients[nfds].is_authenticated = false;
                    
                    printf("New client connected (fd: %d)\n", new_fd);
                    nfds++;
                } else {
                    printf("Max clients reached. Rejecting connection.\n");
                    close(new_fd);
                }
            }
        }

        // Communication received from an existing client
        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                char buffer[1024];
                ssize_t bytes_read = read(fds[i].fd, buffer, sizeof(buffer) - 1);

                if (bytes_read <= 0) {
                    // User logging out (or error)
                    if (clients[i].is_authenticated) {
                        char offline_msg[1024];
                        snprintf(offline_msg, sizeof(offline_msg), "[Server]: %s has gone offline from #%s.\n", clients[i].username, clients[i].channel);
                        
                        for (int j = 1; j < nfds; j++) {
                            if (j != i && clients[j].is_authenticated) {
                                write(fds[j].fd, offline_msg, strlen(offline_msg));
                            }
                        }
                    }

                    printf("Client disconnected (fd: %d)\n", fds[i].fd);
                    close(fds[i].fd);
                    
                    // Tight packing (remove disconnected client and shift the others)
                    fds[i] = fds[nfds - 1];
                    clients[i] = clients[nfds - 1];
                    nfds--;
                    i--;
                } else {
                    // Null terminate the string so we can print it safely regardless
                    buffer[bytes_read] = '\0';
                    buffer[strcspn(buffer, "\r\n")] = '\0';

                    // User authentication check
                    if (!clients[i].is_authenticated) {
                        
                        // User trying to log in (automized process with the GUI)
                        if (strncmp(buffer, "/login ", 7) == 0) {
                            char new_username[MAX_USERNAME];
                            
                            // Username extraction (single-word)
                            if (sscanf(buffer + 7, "%31s", new_username) == 1) {
                                // Duplicate username check
                                bool username_taken = false;
                                
                                // Loop through all active clients to check for a match
                                for (int j = 1; j < nfds; j++) {
                                    if (clients[j].is_authenticated && strcmp(clients[j].username, new_username) == 0) {
                                        username_taken = true;
                                        break; // Stop searching once we find a duplicate
                                    }
                                }

                                if (username_taken) {
                                    // Reject the login attempt
                                    char *err = "[Server]: Username is already taken. Please choose another.\n";
                                    write(fds[i].fd, err, strlen(err));
                                    printf("Server Log: fd %d attempted to use taken username '%s'\n", fds[i].fd, new_username);
                                } else {
                                    // Username not taken, proceed
                                    strncpy(clients[i].username, new_username, MAX_USERNAME);
                                    clients[i].is_authenticated = true;
                                    strncpy(clients[i].channel, "General", MAX_CHANNEL); // Default channel -> Initial login at #General
                                    
                                    // Send a welcome message back to the new client
                                    char welcome_msg[1024];
                                    snprintf(welcome_msg, sizeof(welcome_msg), "[Server]: Welcome to MiniCord, %s!\n", clients[i].username);
                                    write(fds[i].fd, welcome_msg, strlen(welcome_msg));
                                    
                                    printf("Server Log: fd %d authenticated as %s\n", fds[i].fd, clients[i].username);

                                    // Alert all other authenticated users that someone joined
                                    char online_msg[1024];
                                    snprintf(online_msg, sizeof(online_msg), "[Server]: %s has come online in #%s.\n", clients[i].username, clients[i].channel);
                                    
                                    for (int j = 1; j < nfds; j++) {
                                        if (j != i && clients[j].is_authenticated) {
                                            write(fds[j].fd, online_msg, strlen(online_msg));
                                        }
                                    }
                                }
                            } else {
                                char *err = "[Server]: Usage: /login <username>\n";
                                write(fds[i].fd, err, strlen(err));
                            }
                        } else {
                            // Reject any other commands if they haven't logged in yet
                            char *err = "[Server]: You must authenticate first. Type: /login <username>\n";
                            write(fds[i].fd, err, strlen(err));
                        }
                        
                    } else {
                        // User already authenticated, now they have access to regular commands

                        // Unicast (DM)
                        if (strncmp(buffer, "/msg ", 5) == 0) {
                            char target_user[MAX_USERNAME];
                            char *msg_content = buffer + 5; // Advance pointer past "/msg "
                            
                            // Extract the target username
                            if (sscanf(msg_content, "%31s", target_user) == 1) {
                                // Move the msg_content pointer past the username to get the actual message
                                msg_content += strlen(target_user);
                                
                                // Iniital whitespace doesn't matter
                                while (*msg_content == ' ') {
                                    msg_content++;
                                }
                                
                                bool user_found = false;
                                // Finding the target user
                                for (int j = 1; j < nfds; j++) {
                                    if (clients[j].is_authenticated && strcmp(clients[j].username, target_user) == 0) {
                                        char dm_msg[1024];
                                        snprintf(dm_msg, sizeof(dm_msg), "[DM from %s]: %s\n", clients[i].username, msg_content);
                                        write(fds[j].fd, dm_msg, strlen(dm_msg));
                                        user_found = true;
                                        
                                        printf("Server Log - Unicast from %s to %s\n", clients[i].username, target_user);
                                        break; // Stop searching once we find the user
                                    }
                                }
                                
                                if (!user_found) {
                                    char *err = "[Server]: User not found or not logged in.\n";
                                    write(fds[i].fd, err, strlen(err));
                                }
                            } else {
                                char *err = "[Server]: Usage: /msg <username> <message>\n";
                                write(fds[i].fd, err, strlen(err));
                            }
                        } else if (strncmp(buffer, "/join ", 6) == 0) {
                            // Channel subscription
                            char new_channel[MAX_CHANNEL];
                            
                            if (sscanf(buffer + 6, "%31s", new_channel) == 1) {
                                // We need the old channel temporarily for the informative message
                                char old_channel[MAX_CHANNEL];
                                strncpy(old_channel, clients[i].channel, MAX_CHANNEL);
                                
                                // Update the client's state to the new channel
                                strncpy(clients[i].channel, new_channel, MAX_CHANNEL);
                                
                                // Confirmation
                                char join_msg[1024];
                                snprintf(join_msg, sizeof(join_msg), "[Server]: You have joined channel '%s'.\n", clients[i].channel);
                                write(fds[i].fd, join_msg, strlen(join_msg));
                                
                                printf("Server Log - %s moved from #%s to #%s\n", clients[i].username, old_channel, clients[i].channel);

                                // To be able to update the user list, a message is broadcasted to all users to alert them of the channel switch
                                char switch_msg[1024];
                                snprintf(switch_msg, sizeof(switch_msg), "[Server]: %s has switched from #%s to #%s.\n", 
                                         clients[i].username, old_channel, clients[i].channel);
                                
                                for (int j = 1; j < nfds; j++) {
                                    if (j != i && clients[j].is_authenticated) {
                                        write(fds[j].fd, switch_msg, strlen(switch_msg));
                                    }
                                }
                            } else {
                                // Standardized error format
                                char *err = "[Server]: Usage: /join <channel_name>\n";
                                write(fds[i].fd, err, strlen(err));
                            }                               
                        } else if (strncmp(buffer, "/users ", 7) == 0) {
                            // This command is how the GUI can keep a proper track of the user list
                            char target_channel[MAX_CHANNEL];
                            
                            // Extract the requested channel name
                            if (sscanf(buffer + 7, "%31s", target_channel) == 1) {
                                
                                // Standardized message format
                                char roster_msg[1024]; 
                                snprintf(roster_msg, sizeof(roster_msg), "[Server]: Users in #%s: ", target_channel);
                                
                                bool found_users = false;
                                
                                // Loop through all active clients to find who is in the target channel
                                for (int j = 1; j < nfds; j++) {
                                    if (clients[j].is_authenticated && strcmp(clients[j].channel, target_channel) == 0) {
                                        // Concatenate the username and a space to our roster message
                                        strncat(roster_msg, clients[j].username, sizeof(roster_msg) - strlen(roster_msg) - 1);
                                        strncat(roster_msg, " ", sizeof(roster_msg) - strlen(roster_msg) - 1);
                                        found_users = true;
                                    }
                                }

                                strncat(roster_msg, "\n", sizeof(roster_msg) - strlen(roster_msg) - 1);
                                
                                if (found_users) {
                                    write(fds[i].fd, roster_msg, strlen(roster_msg));
                                } else {
                                    char *err = "[Server]: Channel is empty or does not exist.\n";
                                    write(fds[i].fd, err, strlen(err));
                                }
                                
                                printf("Server Log - %s requested roster for channel %s\n", clients[i].username, target_channel);
                                
                            } else {
                                char *err = "Usage: /users <channel_name>\n";
                                write(fds[i].fd, err, strlen(err));
                            } 
                        } else {
                            // Multicast (Channel Broadcast)
                            char broadcast_msg[1024];
                            snprintf(broadcast_msg, sizeof(broadcast_msg), "[%s]: %s\n", clients[i].username, buffer);

                            // Loop through all currently tracked file descriptors and...
                            for (int j = 1; j < nfds; j++) {
                                // ...send the users who are both authenticated and are in the same channel the message
                                if (j != i && clients[j].is_authenticated && 
                                    strcmp(clients[j].channel, clients[i].channel) == 0) {
                                    write(fds[j].fd, broadcast_msg, strlen(broadcast_msg));
                                }
                            }
                            printf("Server Log - Broadcasted in Channel %s: %s", clients[i].channel, broadcast_msg);
                        }
                    }
                }
            }
        }
    }
    return 0;
}