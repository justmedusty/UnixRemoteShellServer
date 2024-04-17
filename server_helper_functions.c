//
// Created by dustyn on 4/17/24.
//

#include "server_helper_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>
//The port from which the client will connect to
#define PORT "6969"
//The backlog will define how many connections can be in the queue at a time
#define backlog 10
#define BUFFER_SIZE 1024

//Pointer function to get the socket address whether it is an IPv4 or IPv6 addr
void *get_in_addr(struct sockaddr *sockAddr) {

    //If this is of family AF_INET (IPv4) we want to grab the sin_addr of this socket (and cast it to the
    if (sockAddr->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sockAddr)->sin_addr);
    }

    //Else we want to grab the sin6_addr which is the IPv6 addr
    return &(((struct sockaddr_in6 *) sockAddr)->sin6_addr);
}

//This function will be used to get our listener socket which will listen for incoming connections on client sockets
int get_listener_socket(void) {

    //This will hold the listener socket file descriptor
    int listener;

    //This will hold the yes value for when we set socket options below
    int yes = 1;

    //This will hold the return value from the getaddrinfo call
    int returnValue;

    //This addrinfo struct , we will apply some values to below and make the system call getaddr info which the kernel will then fill addressInfo struct with all of the available addresses
    //we can bind to this socket, pointer will use for iteration through the result set
    struct addrinfo hints, *addressInfo, *pointer;

    //Make sure the memory is cleared to 0 across the board so we have a blank slate to work with
    memset(&hints, 0, sizeof hints);

    //As usual, our hints we will set family to AF_UNSPEC which just indicates that we don't care about ipv6 or ipv4
    hints.ai_family = AF_UNSPEC;

    //This address info passive flag just means we are asking the kernel to fill in address info for us, for we are lazy!
    hints.ai_flags = AI_PASSIVE;

    //Stream socket, since TCP makes sense for a telnet chat server
    hints.ai_socktype = SOCK_STREAM;

    //Here we will make the getaddrinfo system call which will fill addressInfo with our result set !, handle errors accordingly. We can use the gai_strerror (get address info string error) function to get the string representation of the int error code ! very helpful
    if ((returnValue = getaddrinfo(NULL, PORT, &hints, &addressInfo) != 0)) {
        fprintf(stderr, "select server %s \n", gai_strerror(returnValue));
        exit(EXIT_FAILURE);

    }

    //Walk through the result set pointed to by our result set and try to create the listener socket with each given entry in the list, keep going until we get a winner
    for (pointer = addressInfo; pointer != NULL; pointer = pointer->ai_next) {
        listener = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);
        if (listener < 0) {
            continue;
        }
        //we will set sock option flag SO_REUSEADDR so that we do not get any "address in use" complaints!
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        //If all has succeeded so far, we will attempt to bind our new socket to the corresponding address as specified in the result set.
        if (bind(listener, pointer->ai_addr, pointer->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        // We are handling errors appropriately along the way of course. Lastly , we will make a call to inet_ntop() to get our
        //address in string form as opposed to binary and then print it to the screen.
        char serverIp[INET6_ADDRSTRLEN];
        inet_ntop(pointer->ai_addr->sa_family, get_in_addr((struct sockaddr *) &pointer->ai_addr), serverIp,
                  INET6_ADDRSTRLEN);
        printf("Server IP is %s\n", serverIp);
        break;
    }

    //Free the result set since we created and bound a socket!

    freeaddrinfo(addressInfo);

    //Check that the bind system call worked properly , exit on error code if not
    if (pointer == NULL) {
        fprintf(stderr, "Error occurred during bind process");
        close(EXIT_FAILURE);
    }

    //Here we will call the listener system call which will listen for connections on the given socket , and pass the backlog as our backlog defined above, this is the max queue length for connections.
    //As always, handle errors accordingly
    if (listen(listener, backlog) != 0) {
        fprintf(stderr, "Error on call to listen() with fd %d", listener);
        exit(EXIT_FAILURE);
    }

    //Return the file descriptor of our new listener socket!
    return listener;

}

//This will be a function for adding a new file descriptor to our set to be polled, dynamically allocate memory as needed using realloc to reallocate new memory when we run out of space
void add_to_pfds(struct pollfd *pollFileDescriptors[], int newFd, int *fd_count, int *fd_size) {
    //Double the size if we have run out of space
    if (*fd_count == *fd_size) {
        *fd_size *= 2;
        //Reallocate the new size (double what it previously was)
        *pollFileDescriptors = realloc(*pollFileDescriptors, sizeof(**pollFileDescriptors) * (*fd_size));
    }

    //Add our new file descriptor in this place
    (*pollFileDescriptors)[*fd_count].fd = newFd;
    (*pollFileDescriptors)[*fd_count].events = POLLIN;
    //Iterate the count so that we will be able to iterate over this file descriptor later
    (*fd_count)++;

}

//We'll delete a poll by moving it to the end of the array and reducing the size by 1, you could also just set the fd to a negative value and it would be ignored
void del_pfds(struct pollfd pollFileDescriptors[], int i, int *fd_count) {
    pollFileDescriptors[i] = pollFileDescriptors[*fd_count - 1];
    (*fd_count)--;
}
