//
// Created by dustyn on 4/17/24.
//
#include "poll.h"
#include "sys/socket.h"
#ifndef UNIXREMOTESHELLSERVER_SERVERHELPERFUNCTIONS_H
#define UNIXREMOTESHELLSERVER_SERVERHELPERFUNCTIONS_H
void del_pfds(struct pollfd pollFileDescriptors[], int i, int *fd_count);
void add_to_pfds(struct pollfd *pollFileDescriptors[], int newFd, int *fd_count, int *fd_size);
int get_listener_socket(void);
void *get_in_addr(struct sockaddr *sockAddr);
#endif //UNIXREMOTESHELLSERVER_SERVERHELPERFUNCTIONS_H
