#include "utility.h"

extern server_node server;

void clear(char *net, char *connect_ip, char *connect_port)
{
    char message[13] = "", buff[8] = "";
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (i < 10)
            sprintf(message, "UNREG %s 0%d", net, i);
        else
            sprintf(message, "UNREG %s %d", net, i);
        UDP_server_message(message, buff, sizeof(buff), connect_ip, atoi(connect_port));
        if (strcmp(buff, "OKUNREG") != 0)
            exit(1);
    }
    printf("acabei\n");
}

void show(char *net, char *connect_ip, char *connect_port)
{
    char message[13] = "", buff[1024] = "";

    sprintf(message, "NODES %s", net);
    UDP_server_message(message, buff, sizeof(buff), connect_ip, atoi(connect_port));
    printf("%s", buff);
}

int node_list(char *net, node_t *nodes, char *connect_ip, char *connect_port)
{
    char buff[1024] = "";
    char node_msg[10] = "";

    sprintf(node_msg, "NODES %s", net);
    UDP_server_message(node_msg, buff, sizeof(buff), connect_ip, atoi(connect_port));

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
        return -1; // error: memory allocation failed

    char *delete = nodes_copy;
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
    free(delete);
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

void inicialize_nodes(node_t *nodes)
{
    for (int i = 0; i < MAX_NODES; i++)
    {
        strcpy(nodes[i].id, "\0");
        strcpy(nodes[i].ip, "\0");
        strcpy(nodes[i].port, "\0");
    }
}

int check_input_format(char *input, char *message)
{
    char num1[4], num2[3], num3[3], ip[16];
    int num_dots = 0;

    if (strcmp(message, "get") != 0)
    {
        // Extract the first number
        if (sscanf(input + strlen(message) + 1, "%s", num1) != 1)
        {
            return 0;
        }
        // Check if the first number has exactly 3 digits
        if (strlen(num1) != 3)
        {
            return 0;
        }
        // Check if the first number is all digits
        for (int i = 0; i < strlen(num1); i++)
        {
            if (!isdigit(num1[i]))
            {
                return 0;
            }
        }
        // Extract the second number
        if (sscanf(input + strlen(message) + 1 + strlen(num1) + 1, "%s", num2) != 1)
        {
            return 0;
        }
        // Check if the second number has exactly 2 digits
        if (strlen(num2) != 2)
        {
            return 0;
        }
        // Check if the second number is all digits
        for (int i = 0; i < strlen(num2); i++)
        {
            if (!isdigit(num2[i]))
            {
                return 0;
            }
        }
        if (strcmp(message, "djoin") == 0)
        {
            // Extract the third number
            if (sscanf(input + strlen(message) + 1 + strlen(num1) + 1 + strlen(num2) + 1, "%s", num3) != 1)
            {
                return 0;
            }
            // Check if the third number has exactly 2 digits
            if (strlen(num3) != 2)
            {
                return 0;
            }
            // Check if the third number is all digits
            for (int i = 0; i < strlen(num3); i++)
            {
                if (!isdigit(num3[i]))
                {
                    return 0;
                }
            }
            // Extract the IP address
            if (sscanf(input + strlen(message) + 1 + strlen(num1) + 1 + strlen(num2) + 1 + strlen(num3) + 1, "%15s", ip) != 1)
            {
                return 0;
            }
            // Check if the IP address contains only digits, dots, and has at most 3 digits between each dot
            char *tok;
            tok = strtok(ip, ".");
            while (tok != NULL)
            {
                num_dots++;
                // Check if the token is all digits
                for (int i = 0; i < strlen(tok); i++)
                {
                    if (!isdigit(tok[i]))
                    {
                        return 0;
                    }
                }
                // Check if the token is between 0 and 255
                int num = atoi(tok);
                if (num < 0 || num > 255)
                {
                    return 0;
                }
                tok = strtok(NULL, ".");
            }
            num_dots--;
            if (num_dots != 3)
            {
                return 0;
            }
        }
    }
    else
    {
        // Extract the number
        if (sscanf(input + 4, "%s", num2) != 1)
        {
            return 0;
        }
        // Check if the number has exactly 2 digits
        if (strlen(num2) != 2)
        {
            return 0;
        }
        // Check if the number is all digits
        for (int i = 0; i < strlen(num2); i++)
        {
            if (!isdigit(num2[i]))
            {
                return 0;
            }
        }
    }

    return 1;
}