//
// Created by dustyn on 4/17/24.
//

#include <stdio.h>
#include <sys/wait.h>
#include "HandleClientShellSession.h"
#include "unistd.h"
#include "stdlib.h"
#include "sys/poll.h"
#include "sys/socket.h"

#define BUFFER_SIZE 1024


void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];

    // Create pipes for communication between shell process and client
    int to_shell[2];
    int from_shell[2];
    if (pipe(to_shell) == -1 || pipe(from_shell)== -1){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork a child process to run the shell
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process (shell)
        close(to_shell[1]);   // Close write end of to_shell pipe
        close(from_shell[0]);// Close read end of from_shell pipe

        // Redirect stdin and stdout to the pipes
        if (dup2(to_shell[0], STDIN_FILENO) == -1 || dup2(from_shell[1], STDOUT_FILENO) == -1 || dup2(from_shell[1], STDERR_FILENO) == -1) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }

        // Execute the shell
        execlp("/bin/sh", "/bin/sh", NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(to_shell[0]);   // Close read end of to_shell pipe
        close(from_shell[1]); // Close write end of from_shell pipe


        // Create a pollfd for polling both the client socket and the shell stdout
        struct pollfd fds[3];
        fds[0].fd = client_fd;
        fds[0].events = POLLIN;
        fds[1].fd = from_shell[0];
        fds[1].events = POLLIN;


        while (1) {
            // Poll for events
            int ret = poll(fds, 2, -1);
            if (ret == -1) {
                perror("poll");
                exit(EXIT_FAILURE);
            }

            // Check for events on client socket
            if (fds[0].revents & POLLIN) {
                ssize_t num_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
                if (num_received == -1) {
                    perror("recv");
                    exit(EXIT_FAILURE);
                } else if (num_received == 0) {
                    // Client closed connection
                    break;
                }

                int write_index = 0;
                for (int i = 0; i < num_received; i++) {
                    if (buffer[i] != '\r') {
                        buffer[write_index++] = buffer[i];
                    }
                }

                // Write data to shell process's stdin
                ssize_t num_written = write(to_shell[1], buffer, num_received);
                if (num_written == -1) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }

            // Check for events on shell stdout
            if (fds[1].revents & POLLIN) {
                ssize_t num_read = read(from_shell[0], buffer, BUFFER_SIZE);
                if (num_read == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                } else if (num_read == 0) {
                    // Shell closed stdout
                    break;
                }

                // Send data to client
                ssize_t num_sent = send(client_fd, buffer, num_read, 0);
                if (num_sent == -1) {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
            }

            if (fds[2].revents & POLLIN) {
                ssize_t num_read = read(from_shell[0], buffer, BUFFER_SIZE);
                if (num_read == -1) {
                    perror("read");
                    exit(EXIT_FAILURE);
                } else if (num_read == 0) {
                    // Shell closed stderr
                    break;
                }

                // Send data to client
                ssize_t num_sent = send(client_fd, buffer, num_read, 0);
                if (num_sent == -1) {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
            }
        }

        // Close client socket and wait for shell process to terminate
        close(client_fd);
        close(to_shell[1]);
        close(from_shell[0]);
        wait(NULL); // Wait for shell process to terminate
    }
}