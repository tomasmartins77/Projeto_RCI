#ifndef CONNECTIONS_H
#define CONNECTIONS_H

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

int UDP_server_message(char *, char *, int, char *, int);

void UDP_connection(char *, char *, int, char *, int);

int tcp_client(char *, int);

int create_server(char *, int);

void server_creation();

#endif