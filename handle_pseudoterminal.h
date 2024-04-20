//
// Created by dustyn on 4/20/24.
//
#include "pty.h"
#include "ttyent.h"
#include "unistd.h"
#include "stdlib.h"
#include "stdio.h"
#ifndef UNIXREMOTESHELLSERVER_HANDLE_PSEUDOTERMINAL_H
#define UNIXREMOTESHELLSERVER_HANDLE_PSEUDOTERMINAL_H
int handle_pseudoterminal(int *master,int *slave);
#endif //UNIXREMOTESHELLSERVER_HANDLE_PSEUDOTERMINAL_H
