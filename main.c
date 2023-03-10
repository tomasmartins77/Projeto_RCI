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
typedef struct server_node
{
    struct node VE;
    struct node VB;
    struct node VI[MAX_NODES];
    struct node my_node;
} server_node;

char buffer[1024];

char *UDP_server_message(const char *message, int print);

int handle_join(char *net, char *id, char *ip, char *port, int position, int *client_fds);

void handle_leave(char *net, char *id);

int handle_djoin(char *net, char *id, char *bootid, char *bootIP, char *bootTCP);

void handle_create(char *name);

void handle_delete(char *name);

void handle_get(char *dest, char *name);

void handle_st(char *net);

void handle_sn(char *net);

void handle_sr(char *net);

int node_list(char *net, int print);

int parse_nodes(char *nodes_str, int max_nodes);

int verify_node(char *id, int count);

void inicialize_node();

char *random_number(char new_str[3]);

int tcp_connect(int num_nodes);

int tcp_client(char *ip_address, int portno, char *message, char *response);

void clear(char *net);

node_t nodes[MAX_NODES];
server_node server;

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int position = 0;
    fd_set rfds_list;
    int keyfd = 0, count = 0, flag = 0;
    char buff[1024], str_temp[10], id_temp[3], ip_temp[16], port_temp[6];
    char message[10], arg1[9], arg2[5], bootid[7], bootIP[7], bootTCP[8];
    node_t temp;
    int server_fd, client_fds[MAX_NODES] = {-1};
    struct sockaddr_in server_addr, client_addr;
    socklen_t cli_addr_size = sizeof(client_addr);

    inicialize_node();

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

    while (1)
    {
        FD_ZERO(&rfds_list);           // poem todos a 0
        FD_SET(keyfd, &rfds_list);     // adiciona o keyboard
        FD_SET(server_fd, &rfds_list); // adiciona o server

        int ready = select(MAX_NODES + 1, &rfds_list, NULL, NULL, NULL); // ve se o keyboard foi set ou n
        if (ready < 0)
            /*error*/ exit(1);

        if (FD_ISSET(keyfd, &rfds_list) == 1)
        {
            fgets(buff, 255, stdin); // LE o que ta escrito
            sscanf(buff, "%s %s %s %s %s %s", message, arg1, arg2, bootid, bootIP, bootTCP);
            if (strcmp(message, "join") == 0)
            {
                count = handle_join(arg1, arg2, argv[1], argv[2], position, client_fds);

                if (count > 0)
                    FD_SET(client_fds[position++], &rfds_list);
                flag = 1;
            }
            if (strcmp(message, "leave") == 0 && flag == 1)
                handle_leave(arg1, server.my_node.id);
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
            if (strcmp(message, "clear") == 0)
            {
                clear(arg1);
                exit(1);
            }
        }
        if (FD_ISSET(server_fd, &rfds_list) == 1)
        {
            if ((client_fds[position] = accept(server_fd, (struct sockaddr *)&client_addr, &cli_addr_size)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            FD_SET(client_fds[position++], &rfds_list);
        }
        for (int x = 0; x < (position - 1); x++)
        {
            if (FD_ISSET(client_fds[x], &rfds_list) == 1)
            {
                memset(buff, 0, 1024);
                read(client_fds[x], buff, 1024);
                fprintf(stdout, "%s", buff);
                sscanf(buff, "%s %s %s %s", str_temp, id_temp, ip_temp, port_temp);

                if (strcmp(str_temp, "NEW") == 0)
                {
                    if (strcmp(server.my_node.id, server.VE.id) == 0) // ancora
                    {
                        strcpy(server.VE.id, id_temp);
                        strcpy(server.VE.ip, ip_temp);
                        strcpy(server.VE.port, port_temp);
                    }
                    else
                    {
                        printf("nao\n");
                        // colocar como interno
                    }

                    sprintf(buff, "EXTERN %s %s %s\n", server.VE.id, server.VE.ip, server.VE.port);
                    write(client_fds[x], buff, 1024);
                }
                if (strcmp(str_temp, "EXTERN") == 0)
                {
                    strcpy(server.VB.id, id_temp);
                    strcpy(server.VB.ip, ip_temp);
                    strcpy(server.VB.port, port_temp);
                }
            }
        }
    }
    return 0;
}

int handle_join(char *net, char *id, char *ip, char *port, int position, int *client_fds)
{
    int flag = 0;
    char id_connect[3];
    char message[50];
    int count = node_list(net, 0);

    strcpy(server.my_node.id, id);
    strcpy(server.my_node.ip, ip);
    strcpy(server.my_node.port, port);
    strcpy(id_connect, id);
    if (count > 0)
    {
        while (verify_node(id_connect, count) == 0)
            strcpy(id_connect, random_number(id_connect));
        strcpy(server.my_node.id, id_connect);
        client_fds[position] = tcp_connect(count);
    }
    strcpy(server.VE.id, server.my_node.id);
    strcpy(server.VB.id, server.my_node.id);

    sprintf(message, "REG %s %s %s %s", net, id_connect, ip, port);
    if (strcmp(UDP_server_message(message, 1), "OKREG") != 0)
        exit(1);

    node_list(net, 1);
    return count;
}

int verify_node(char *id, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(nodes[i].id, id) == 0)
            return 0;
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

int handle_djoin(char *net, char *id, char *bootid, char *bootIP, char *bootTCP)
{
    char message[20];
    sprintf(message, "REG %s %s", net, id);
    int fd;
    char response[20];

    sprintf(message, "New %s %s %s", id, bootIP, bootTCP);
    fd = tcp_client(bootid, atoi(bootTCP), message, response);
    return fd;
}

int tcp_client(char *ip_address, int portno, char *message, char *response)
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer_local[256];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }
    server = gethostbyname(ip_address);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error connecting");
        exit(1);
    }
    n = write(sockfd, message, strlen(message));
    if (n < 0)
    {
        perror("Error writing to socket");
        exit(1);
    }
    bzero(buffer_local, 256);
    n = read(sockfd, buffer_local, 255);
    if (n < 0)
    {
        perror("Error reading from socket");
        exit(1);
    }
    strcpy(response, buffer_local);
    return sockfd;
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
        fprintf(stdout, "%s\n", buffer);
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
}

char *random_number(char new_str[3])
{
    int number = rand() % 100;
    sprintf(new_str, "%02d", number);
    return new_str;
}

int tcp_connect(int num_nodes)
{
    int int_connect = rand() % num_nodes, sock = 9;
    char ip_connect[16], message[255];
    char port_connect[6];
    struct sockaddr_in serv_addr;
    strcpy(ip_connect, nodes[int_connect].ip);
    strcpy(port_connect, nodes[int_connect].port);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stdout, "\n Socket creation error \n");
        exit(-1);
    }
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port_connect));

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_connect, &serv_addr.sin_addr) <= 0)
    {
        fprintf(stdout, "\nInvalid address/ Address not supported \n");
        exit(-1);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stdout, "\nConnection Failed \n");
        exit(-1);
    }
    sprintf(buffer, "NEW %s %s %s\n", server.my_node.id, server.my_node.ip, server.my_node.port);
    write(sock, buffer, strlen(buffer));
    return sock;
}

void clear(char *net)
{
    node_list(net, 1);
    char message[13];
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (i < 10)
            sprintf(message, "UNREG %s 0%d", net, i);
        else
            sprintf(message, "UNREG %s %d", net, i);
        UDP_server_message(message, 1);
    }
    printf("acabei\n");
    node_list(net, 1);
}