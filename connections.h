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

void UDP_server_message(char *message, char *response, int len);

int tcp_client(char *ip_address, int portno);

int create_server(char *ip_address, int port);

#endif