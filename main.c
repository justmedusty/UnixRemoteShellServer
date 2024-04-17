


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <poll.h>
#include "HandleClientShellSession.h"
#include "ServerHelperFunctions.h"
//The port from which the client will connect to
#define PORT "6969"
//The backlog will define how many connections can be in the queue at a time
#define backlog 10
#define BUFFER_SIZE 1024

/*
 * Now polling is important, it is one of 3 ways to handle incoming messages on your sockets
 * by default when iterating through sockets, it will block indefinately until you get something
 * which is obviously not ideal at all. Polling allows you to wait for an event that lets you know
 * there is something ready to be read, or wrote depending on your needs. Here we will go through a
 * basic telnet server which users can telnet to and each message they send is broadcast to all other members
 * in the chat currently. we will use poll() to handle the blocking problem. You can also use select() or epoll().
 * Epoll is linux specific whereas poll and select are standards across unix-like operating systems
 *
 */




//Our main function where our telnet server loop will happen
int main(void) {

    //Our listener file descriptor which of which the listener socket will listen for connections for us
    int listener;
    //This newFd will be assigned dynamically as each POLLIN event happens
    int newFd;
    //This will also be assigned dynamically as each POLLIN event happens with a new telnet connection
    struct sockaddr_storage clientAddress;
    //This socklength type will store the length of the address
    socklen_t addrlen;

    //The buffer which will store the messages that are being
    char buffer[1024];
    //This will store our client address, we allocate size of INET6_ADDRSTRLEN because it is big enough to hold either IPv4 or IPv6
    char clientIP[INET6_ADDRSTRLEN];
    //Our count will start with 0 since we don't have any polling file descriptors yet
    int fd_count = 0;
    //Our initial size will be for 5 polling file descriptors
    int fd_size = 5;
    //Now we instantiate and allocate our array dynamically of size specified above (5 pollfds)
    struct pollfd *pollFileDescriptors = malloc(sizeof(*pollFileDescriptors) * fd_size);

    //We will assign the listener fd the result of our get_listener_socket() which we wrote above, this will spit out a listener for us
    listener = get_listener_socket();

    //Error handle to check if listener socket was created and bound properly
    if (listener == -1) {
        fprintf(stderr, "Error creating listener during call to get_listener_socket() \n");
        exit(EXIT_FAILURE);
    }

    //No we add it to our polling file descriptor array
    pollFileDescriptors[0].fd = listener;

    //We will assign event POLLIN which means we want to know when there is an incoming connection on this socket
    pollFileDescriptors[0].events = POLLIN, POLL_ERR,POLL_HUP;

    //Set our fd_count to 1 since we have just added our listener
    fd_count = 1;

    //Here we will start an infinite loop, for(;;) is shorthand for an infinite loop in C
    for (;;) {
        //We will make the poll system call, this is where we will actually start the polling for return events, timeout-1 to indicate that we do not want a timeout, go forever !
        int poll_count = poll(pollFileDescriptors, fd_size, -1);

        //Standard error checking, if return value is -1 it means an error happened
        if (poll_count == -1) {
            perror("Poll sys call failed");
            exit(EXIT_FAILURE);
        }

        //We will start our first loop on all polling file descriptors in the array
        for (int i = 0; i < fd_count; i++) {

            //If the return event is POLLIN...
            if (pollFileDescriptors[i].revents & POLLIN) {

                //We will check if it is the listener, POLLIN from the listener means that someone is ready to connect to our telnet server!
                if (pollFileDescriptors[i].fd == listener) {

                    //we will assign addr len to the length of clientAddress so there is enough space
                    addrlen = sizeof clientAddress;
                    //We will accept the connection that the listener is ready to give us, and we will feed it the address of clientAddr and addr len in order to
                    //let it fill in these values with the values from the client in  this new connection
                    newFd = accept(listener, (struct sockaddr *) &clientAddress, &addrlen);

                    //Standard error checking, if newFd is -1 , houston we have a problem
                    if (newFd == -1) {
                        perror("accept() system call");
                    } else {
                        //Otherwise add to our polling file descriptors list with a call to our add_to_pfds call above,which will add it to the list , dynamically handle size, and increment the count for us!
                        add_to_pfds(&pollFileDescriptors, newFd, &fd_count, &fd_size);

                        //Print out the new connection to the console with the ip address converted from network (binary) to presentation (string) via the inet_ntop() system call
                        printf("Poll server : new connection from %s  on socket %d\n",

                               inet_ntop(clientAddress.ss_family,
                                         get_in_addr((struct sockaddr *) &clientAddress), clientIP, INET6_ADDRSTRLEN),
                               newFd);

                    }

                    //Else it is NOT the listener, and it is a client waiting to send a message to the telnet server
                } else {
                    if(pollFileDescriptors[i].fd == newFd){
                        int pid = fork();
                        if (pid == -1) {
                            perror("fork");
                            exit(EXIT_FAILURE);
                        }
                        if (pid == 0) {
                            handle_client(newFd);
                            exit(EXIT_SUCCESS); // exit child process after handling client
                        } else {
                            newFd = -1;
                            close(pollFileDescriptors[i].fd);
                            del_pfds(pollFileDescriptors, i, &fd_count);
                            continue;

                        }

                    }

                }
                continue;
            }
                // Check for POLLHUP and POLLERR to handle disconnections
            else if (pollFileDescriptors[i].revents & (POLLHUP | POLLERR)) {
                printf("Client on socket %d disconnected\n", pollFileDescriptors[i].fd);
                close(pollFileDescriptors[i].fd);
                del_pfds(pollFileDescriptors, i, &fd_count);

            }
        }
    }
    //Unreachable code here, but we don't give a fuck we'll write it anyway, we're freaks
    return EXIT_SUCCESS;

}




