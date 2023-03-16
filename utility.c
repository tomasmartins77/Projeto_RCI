#include "utility.h"

extern server_node server;

void clear(char *net)
{
    char message[13], buff[8];
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (i < 10)
            sprintf(message, "UNREG %s 0%d", net, i);
        else
            sprintf(message, "UNREG %s %d", net, i);
        UDP_server_message(message, 1, buff, sizeof(buff));
        if (strcmp(buff, "OKUNREG") != 0)
            exit(1);
    }
    printf("acabei\n");
}

void UDP_server_message(char *message, int print, char *response, int len)
{
    int sockfd;
    struct sockaddr_in server_addr;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        exit(1);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); // mudar para receber do terminal!!!!!!!!!!!!!!!!!!!!!!
    server_addr.sin_port = htons(SERVER_PORT);

    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("sendto error\n");
        exit(1);
    }
    int n = recvfrom(sockfd, response, len, 0, NULL, NULL);
    if (n < 0)
    {
        perror("recvfrom error\n");
        exit(1);
    }
    response[n] = '\0';
    if (print == 1)
        fprintf(stdout, "%s\n", response);
    close(sockfd);
}

int node_list(char *net, int print, node_t *nodes)
{
    char buff[1024];
    char node_msg[10];

    sprintf(node_msg, "NODES %s", net);
    UDP_server_message(node_msg, print, buff, sizeof(buff));

    return parse_nodes(buff, nodes);
}

int parse_nodes(char *nodes_str, node_t *nodes)
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

    while (line != NULL && node_count < MAX_NODES)
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

int verify_node(char *id, int count, node_t *nodes)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(nodes[i].id, id) == 0)
        {
            return 0;
        }
    }
    return 1;
}

char *random_number(char new_str[3])
{
    int number = rand() % 100;
    sprintf(new_str, "%02d", number);
    return new_str;
}

int tcp_client(char *ip_address, int portno)
{
    int sockfd;
    struct sockaddr_in serv_addr;

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
