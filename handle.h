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
#include "connections.h"
#include "fd_functions.h"

void handle_leave(char *, char *, char *, char *);

int handle_join(char *, char *, char *, char *);

int handle_djoin(char *, char *, char *, char *, char *, char *, char *);

int handle_create(char *, int);

void handle_delete(char *);

int handle_get(char *, char *, char *, int);

void handle_cr();

void handle_st();

void handle_sn();

void handle_sr();

#endif