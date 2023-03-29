#ifndef FD_FUNCTIONS_H
#define FD_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <time.h>
#include "utility.h"
#include "handle.h"
#include "connections.h"
#include "fd_functions.h"

void withdraw(int, int);

void leave(int x);

fd_set handle_menu(fd_set, char *, char *, char *, char *);

fd_set client_fd_set(fd_set, int);

#endif