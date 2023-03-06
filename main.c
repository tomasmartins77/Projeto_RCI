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

#define PORT 8081
#define MAX_NODES 99
#define SERVER_IP "193.136.138.142" // Change to the IP address of your server
#define SERVER_PORT 59000           // Change to the port number of your server

typedef struct node
{
    char id[3];
    char ip[16];
    char port[6];
} node_t;
typedef struct our_node
{
    int VE;
    int VB;
    int VI[MAX_NODES];
    struct node my_node;
} our_node;

char buffer[1024];

char *UDP_server_message(const char *message, int print);

char *ipv4_find();

int handle_join(char *net, char *id, char *ip, char *port);

void handle_leave(char *net, char *id);

void handle_djoin(char *net, char *id, char *bootid, char *bootIP, char *bootTCP);

void handle_create(char *name);

void handle_delete(char *name);

void handle_get(char *dest, char *name);

void handle_st(char *net);

void handle_sn(char *net);

void handle_sr(char *net);

int node_list(char *net, int print);

int parse_nodes(char *nodes_str, int max_nodes);

int verify_node(char *net, int count);

void inicialize_node();

char *random_number(char new_str[3]);

void tcp_connect(int num_nodes);

node_t nodes[MAX_NODES];
our_node this_node;
int client_fds[MAX_NODES];

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int fd, errcode, i = 0;
    ssize_t n;
    fd_set rfds, rfds_list;
    int keyfd = 0, flag;
    char buff[255];
    char message[10], arg1[9], arg2[5], bootid[7], bootIP[7], bootTCP[8];

    int server_fd, max_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t cli_addr_size = sizeof(client_addr);

    inicialize_node();

    for (int i = 0; i < MAX_NODES; i++)
    {
        client_fds[i] = -1;
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[2]));
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, MAX_NODES) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    max_fd = server_fd;

    while (1)
    {
        FD_ZERO(&rfds);           // poem todos a 0
        FD_SET(keyfd, &rfds);     // adiciona o keyboard
        FD_SET(server_fd, &rfds); // adiciona o server

        rfds_list = rfds;

        int ready = select(MAX_NODES + 1, &rfds_list, NULL, NULL, NULL); // ve se o keyboard foi set ou n
        if (ready < 0)
            /*error*/ exit(1);

        if (FD_ISSET(keyfd, &rfds_list) == 1)
        {
            fgets(buff, 255, stdin); // LE o que ta escrito
            sscanf(buff, "%s %s %s %s %s %s", message, arg1, arg2, bootid, bootIP, bootTCP);

            if (strcmp(message, "join") == 0)
                flag = handle_join(arg1, arg2, argv[1], argv[2]);
            if (strcmp(message, "leave") == 0 && flag == 1)
                handle_leave(arg1, arg2);
            else if (flag == 0)
                fprintf(stdout, "no node created\n");
            if (strcmp(message, "djoin") == 0)
                handle_djoin(arg1, arg2, bootid, bootIP, bootTCP);
            if (strcmp(message, "create") == 0)
                handle_create(arg1);
            if (strcmp(message, "delete") == 0)
                handle_delete(arg1);
            if (strcmp(message, "get") == 0)
                handle_get(arg1, arg2);
            if (strcmp(message, "show topology") == 0 || strcmp(message, "st") == 0)
                handle_st(arg2);
            if (strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0)
                handle_sn(arg2);
            if (strcmp(message, "show routing") == 0 || strcmp(message, "sr") == 0)
                handle_sr(arg2);
            if (strcmp(message, "exit") == 0)
            {
                fprintf(stdout, "exiting program\n");
                exit(1);
            }
        }
        if (FD_ISSET(server_fd, &rfds_list) == 1)
        {
            for (i = 0; i < MAX_NODES; i++)
            {
                if (client_fds[i] != -1)
                    break;
            }
            if ((client_fds[i] = accept(server_fd, (struct sockaddr *)&client_addr, &cli_addr_size)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            FD_SET(client_fds[i], &rfds_list);
        }
        for (int x = 0; x < i; x++)
        {
            if (FD_ISSET(client_fds[x], &rfds_list) == 1)
            {
                memset(buffer, 0, 1024);
                read(client_fds[x], buffer, 1024);
                fprintf(stdout, "%s\n", buffer);

                if (strcmp(buffer, "NEW") == 0)
                {
                    sprintf(buffer, "EXTERN %s %s %s\n", nodes[this_node.VE].id, nodes[this_node.VE].ip, nodes[this_node.VE].port);
                    write(client_fds[x], buffer, 1024);
                }
            }
        }
    }
    return 0;
}

int handle_join(char *net, char *id, char *ip, char *port)
{
    int flag = 0, conn_node = 0;
    char new_id[3], right_node[3];
    char message[50];
    int count = node_list(net, 0);
    char *endptr;

    if (count > 0)
    {
        while (verify_node(id, count) == 0)
        {
            random_number(new_id);
            strcpy(id, new_id);
        }
    }

    sprintf(message, "REG %s %s %s %s", net, id, ip, port);
    if (strcmp(UDP_server_message(message, 1), "OKREG") == 0)
        flag = 1;
    strcpy(this_node.my_node.id, id);
    strcpy(this_node.my_node.ip, ip);
    strcpy(this_node.my_node.port, port);
    long num = strtol(id, &endptr, 10);

    this_node.VB = (int)num;
    if (count > 0)
        tcp_connect(count);

    node_list(net, 1);
    return flag;
}

int verify_node(char *net, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (strcmp(nodes[i].id, net) == 0)
        {
            return 0;
        }
    }
    return 1;
}

int parse_nodes(char *nodes_str, int max_nodes)
{
    char *line;
    char *token;
    int node_count = 0;
    // make a copy of the input string
    char *nodes_copy = strdup(nodes_str);
    if (nodes_copy == NULL)
    {
        // error: memory allocation failed
        return -1;
    }
    // split string into lines
    line = strtok_r(nodes_copy, "\n", &nodes_copy);
    line = strtok_r(NULL, "\n", &nodes_copy);

    while (line != NULL && node_count < max_nodes)
    {
        node_t node;
        // split line into tokens
        token = strtok(line, " ");
        if (token != NULL)
        {
            strncpy(node.id, token, sizeof(node.id));
            node.id[sizeof(node.id) - 1] = '\0';
        }

        token = strtok(NULL, " ");
        if (token != NULL)
        {
            strncpy(node.ip, token, sizeof(node.ip));
            node.ip[sizeof(node.ip) - 1] = '\0';
        }

        token = strtok(NULL, " ");
        if (token != NULL)
        {
            strncpy(node.port, token, sizeof(node.port));
            node.port[sizeof(node.port) - 1] = '\0';
        }

        nodes[node_count++] = node;
        // move to next line
        line = strtok_r(NULL, "\n", &nodes_copy);
    }
    return node_count;
}

void handle_leave(char *net, char *id)
{
    char message[13];
    sprintf(message, "UNREG %s %s", net, id);
    UDP_server_message(message, 1);
    node_list(net, 1);
}

void handle_djoin(char *net, char *id, char *bootid, char *bootIP, char *bootTCP)
{
}

void handle_create(char *name)
{
}

void handle_delete(char *name)
{
}

void handle_get(char *dest, char *name)
{
}

void handle_st(char *net)
{
}

void handle_sn(char *net)
{
}

void handle_sr(char *net)
{
}

int node_list(char *net, int print)
{
    char buff[1024];
    char node_msg[255];
    sprintf(node_msg, "NODES %s", net);
    strcpy(buff, UDP_server_message(node_msg, print));

    return parse_nodes(buff, MAX_NODES);
}

char *ipv4_find()
{
    int n;
    struct ifreq ifr;

    n = socket(AF_INET, SOCK_DGRAM, 0);
    // Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    // Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);

    // display result
    printf("IP Address is %s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
    return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

char *UDP_server_message(const char *message, int print)
{
    int sockfd;
    struct sockaddr_in server_addr;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        exit(1);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);

    if (sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        exit(1);
    }

    int len = sizeof(server_addr);
    int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &len);
    if (n < 0)
    {
        exit(1);
    }
    buffer[n] = '\0';
    if (print == 1)
        printf("Server says: %s\n", buffer);
    close(sockfd);

    return buffer;
}

void inicialize_node()
{
    strncpy(nodes->id, "000", sizeof(nodes->id) - 1);
    strncpy(nodes->ip, "000", sizeof(nodes->ip) - 1);
    strncpy(nodes->port, "00000", sizeof(nodes->port) - 1);
    nodes->id[sizeof(nodes->id) - 1] = '\0';
    nodes->ip[sizeof(nodes->ip) - 1] = '\0';
    nodes->port[sizeof(nodes->port) - 1] = '\0';
    this_node.VE = 0;
    this_node.VB = 0;
    for (int i = 0; i < MAX_NODES; i++)
        this_node.VI[i] = 0;
}

char *random_number(char new_str[3])
{
    int number = rand() % 100;
    sprintf(new_str, "%02d", number);
    return new_str;
}

void tcp_connect(int num_nodes)
{
    int id_connect = rand() % num_nodes;
    char ip_connect[16], message[255];
    char port_connect[6];
    strcpy(ip_connect, nodes[id_connect].ip);
    strcpy(port_connect, nodes[id_connect].port);
    struct sockaddr_in serv_addr;
    int i;
    for (i = 0; i < MAX_NODES; i++)
    {
        if (client_fds[i] != -1)
            break;
    }
    if ((client_fds[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        exit(-1);
    }
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port_connect));

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_connect, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        exit(-1);
    }

    if (connect(client_fds[i], (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        exit(-1);
    }
    sprintf(buffer, "NEW %s %s %s\n", this_node.my_node.id, this_node.my_node.ip, this_node.my_node.port);
    write(client_fds[i], buffer, strlen(buffer));
}