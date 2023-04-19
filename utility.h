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
#define MAX_BUFFER 255
#define SERVER_IP "193.136.138.142" // Change to the IP address of your server
#define SERVER_PORT "59000"         // Change to the port number of your server

typedef struct node
{
    char id[3];              // node id
    char ip[16];             // ip address
    char port[6];            // port number
    int fd;                  // file descriptor of that node
    int active;              // 0 -> inactive, 1 -> active node
    char buffer[MAX_BUFFER]; // buffer to store the data received
    int bytes_received;      // number of bytes received to the buffer so far
} node_t;
typedef struct server_node
{
    struct node vb;                  // backup
    struct node vz[MAX_NODES];       // vz[0] -> extern node, all others -> intern nodes
    struct node my_node;             // my node's info
    char names[MAX_NODES][MAX_NAME]; // names created by the server
    char net[4];                     // network connected to
    int exptable[MAX_NODES];         // expedition table
} server_node;

void clear(char *, char *, char *);

void show(char *, char *, char *);

int node_list(char *, node_t *, char *, char *);

int parse_nodes(char *, node_t *);

int verify_node(char *, int, node_t *);

char *random_number(char *);

int timeout(int, int);

void inicialize_nodes(node_t *);

int check_input_format(char *, char *);

int check_arguments(int, char **, char *, char *);

int isValidIP(char *);

int isValidPort(char *);

#endif /* HEADER_H */
