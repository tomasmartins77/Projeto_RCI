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
#include <sys/time.h>
#include <time.h>
#include <ctype.h>
#include "utility.h"
#include "connections.h"
#include "fd_functions.h"

#define MAX_NODES 100
#define MAX_NAME 100
#define MAX_BUFFER 50
#define SERVER_IP "193.136.138.142" // Change to the IP address of your server
#define SERVER_PORT "59000"         // Change to the port number of your server

typedef struct node
{
    char id[3];
    char ip[16];
    char port[6];
    int fd;
    int active;
    char buffer[MAX_BUFFER];
    int bytes_recieved;
} node_t;
typedef struct server_node
{
    struct node vb;
    struct node vz[MAX_NODES];
    struct node my_node;
    char names[MAX_NODES][MAX_NAME];
    char net[4];
    int exptable[MAX_NODES];
} server_node;

void clear(char *, char *, char *);

void show(char *, char *, char *);

int node_list(char *, node_t *, char *, char *);

int parse_nodes(char *, node_t *);

int verify_node(char *, int, node_t *);

char *random_number(char *);

void timeout(int, int);

void inicialize_nodes(node_t *);

int check_input_format(char *, char *);

#endif /* HEADER_H */
