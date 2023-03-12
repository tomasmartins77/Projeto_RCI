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
} node_t;
typedef struct server_node
{
    struct node VE;
    struct node VB;
    struct node VI[MAX_NODES];
    struct node my_node;
} server_node;

void clear(char *net);

int create_server(char *ip_address, int port);

char *UDP_server_message(const char *message, int print);

int node_list(char *net, int print);

int parse_nodes(char *nodes_str, int max_nodes);

int verify_node(char *net, int count);

char *random_number(char new_str[3]);

int tcp_connect(int num_nodes);

int tcp_client(char *ip_address, int portno, char *message, char *response);

#endif /* HEADER_H */
