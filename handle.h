#ifndef HANDLE_H
#define HANDLE_H

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

void handle_leave(char *net, char *id, int *client_fds);

int handle_join(char *net, char *id, char *ip, char *port, int *socket);

int handle_djoin(char *net, char *id, char *bootid, char *bootIP, char *bootTCP);

int handle_create(char *name);

void handle_delete(char *name);

void handle_get(char *dest, char *name);

void handle_st(int intr);

void handle_sn();

void handle_sr(char *net);

fd_set handle_menu(fd_set rfds_list, int *client_fds, int server_fd, char *ip, char *port, int intr);

fd_set client_fd_set(fd_set rfds_list, int *client_fds, int x, int *intr);

#endif /* HEADER_H */