#include "handle.h"

extern char buffer[1024];
extern node_t nodes[MAX_NODES];
extern server_node server;

int handle_join(char *net, char *id, char *ip, char *port, int position, int *client_fds)
{
    int flag = 0;
    char id_connect[3];
    char message[50];
    node_t temp;
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
    else
        server.VE = server.my_node;
    server.VB = server.my_node;

    sprintf(message, "REG %s %s %s %s", net, id_connect, ip, port);
    if (strcmp(UDP_server_message(message, 1), "OKREG") != 0)
        exit(1);

    node_list(net, 1);
    return count;
}

void handle_leave(char *net, char *id, int position, int *client_fds)
{
    char message[13];
    for (int i = 0; i < (position - 1); i++)
        close(client_fds[i]);

    sprintf(message, "UNREG %s %s", net, id);
    UDP_server_message(message, 1);
    node_list(net, 1);
}

int handle_djoin(char *net, char *id, char *bootid, char *bootIP, char *bootTCP)
{
    char message[20];
    char response[20];
    int fd;

    sprintf(message, "New %s %s %s", id, bootIP, bootTCP);
    fd = tcp_client(bootid, atoi(bootTCP), message, response);
    return fd;
}

int handle_create(char *name)
{
    int i, flag = 0;
    for (i = 0; i < 50; i++)
    {
        if (strcmp(server.names[i], "\0") == 0)
        {
            strcpy(server.names[i], name);
            flag++;
            break;
        }
    }
    fprintf(stdout, "created file: %s\n", server.names[i]);
    return flag;
}

void handle_delete(char *name)
{
    int i, flag = 0;
    for (i = 0; i < 50; i++)
    {
        if (strcmp(server.names[i], name) == 0)
        {
            strcpy(server.names[i], "\0");
            flag = 1;
            fprintf(stdout, "deleted file: %s\n", name);
            break;
        }
    }
    if (flag == 0)
    {
        fprintf(stdout, "no file deleted\n");
    }
}

void handle_get(char *dest, char *name)
{
    /* function code here */
}

void handle_st(int intr)
{
    fprintf(stdout, "Vizinho externo: %s %s %s\n", server.VE.id, server.VE.ip, server.VE.port);
    fprintf(stdout, "Vizinho Backup: %s %s %s\n", server.VB.id, server.VB.ip, server.VB.port);
    fprintf(stdout, "Vizinhos internos:\n");
    for (int i = 0; i < intr; i++)
    {
        fprintf(stdout, "%s %s %s\n", server.VI[i].id, server.VI[i].ip, server.VI[i].port);
    }
}

void handle_sn()
{
    int i;
    fprintf(stdout, "files:\n");
    for (i = 0; i < 50; i++)
    {
        if (strcmp(server.names[i], "\0") != 0)
        {
            fprintf(stdout, "%s\n", server.names[i]);
        }
    }
}

void handle_sr(char *net)
{
    /* function code here */
}