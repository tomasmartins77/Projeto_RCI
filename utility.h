#ifndef UTILITY_H
#define UTILITY_H

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

#define MAX_NODES 99
#define SERVER_IP "193.136.138.142" // Change to the IP address of your server
#define SERVER_PORT 59000           // Change to the port number of your server

typedef struct node
{
    char id[3];
    char ip[16];
    char port[6];
    int fd;
} node_t;
typedef struct server_node
{
    struct node vb;
    struct node vz[MAX_NODES];
    struct node my_node;
    char names[50][100];
    char net[4];
} server_node;

void clear(char *net);

int create_server(char *ip_address, int port);

void UDP_server_message(char *message, int print, char *response, int len);

int node_list(char *net, int print, node_t *nodes);

int parse_nodes(char *nodes_str, node_t *nodes);

int verify_node(char *net, int count, node_t *nodes);

char *random_number(char new_str[3]);

int tcp_client(char *ip_address, int portno);

#endif /* HEADER_H */
