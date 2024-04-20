//
// Created by dustyn on 4/20/24.
//

#include "handle_pseudoterminal.h"

int handle_pseudoterminal(int *master,int *slave){

    char slave_name[128];

    // Create a pseudo-terminal pair
    if (openpty(master, slave, slave_name, NULL, NULL) == -1) {
        perror("openpty");
        return -1;
    }
    return 0;

}