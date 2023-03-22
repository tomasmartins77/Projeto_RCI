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
        UDP_server_message(message, buff, sizeof(buff));
        if (strcmp(buff, "OKUNREG") != 0)
            exit(1);
    }
    printf("acabei\n");
}

int node_list(char *net, int print, node_t *nodes)
{
    char buff[1024];
    char node_msg[10];

    sprintf(node_msg, "NODES %s", net);
    UDP_server_message(node_msg, buff, sizeof(buff));

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
        if (strcmp(nodes[i].id, id) == 0 && strcmp(nodes[i].ip, "0") != 0)
            return 0;
    }
    return 1;
}

char *random_number(char *new_str)
{
    int number = rand() % 100;
    sprintf(new_str, "%02d", number);
    return new_str;
}

void timeout(int time, int socket)
{
    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(socket, &rfds);
    select(socket + 1, &rfds, NULL, NULL, &timeout);
    if (FD_ISSET(socket, &rfds))
        return;
}