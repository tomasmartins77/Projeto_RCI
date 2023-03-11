#include "utility.h"

extern server_node server;

char buffer[1024];
node_t nodes[MAX_NODES];
server_node this_node;
int client_fds[MAX_NODES];

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

int node_list(char *net, int print)
{
    char buff[1024];
    char node_msg[255];
    sprintf(node_msg, "NODES %s", net);
    strcpy(buff, UDP_server_message(node_msg, print));

    return parse_nodes(buff, MAX_NODES);
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

node_t parse_line(char *line)
{
    char *token;
    node_t node;
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
    return node;
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

int tcp_client(char *ip_address, int portno, char *message, char *response)
{
     int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer_local[256];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stdout, "\n Socket creation error \n");
        exit(-1);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0)
    {
        fprintf(stdout, "\nInvalid address/ Address not supported \n");
        exit(-1);
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stdout, "\nConnection Failed \n");
        exit(-1);
    }

    write(sockfd, message, strlen(message));
    read(sockfd,buffer_local,255);
    strcpy(response,buffer_local);
    return sockfd;
}

int create_server(char *ip_address, int port)
{
    int server_fd;
    struct sockaddr_in server_addr;

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
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    server_addr.sin_port = htons(port);
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
    return server_fd;
}
