#include "handle.h"


extern server_node this_node;
extern server_node server;

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
    strcpy(server.VE.ip, server.my_node.ip);
    strcpy(server.VE.port, server.my_node.port);
    strcpy(server.VB.id, server.my_node.id);
    strcpy(server.VB.ip, server.my_node.ip);
    strcpy(server.VB.port, server.my_node.port);

    sprintf(message, "REG %s %s %s %s", net, id_connect, ip, port);
    if (strcmp(UDP_server_message(message, 1), "OKREG") != 0)
        exit(1);

    node_list(net, 1);
    return count;
}

void handle_leave(char *net, char *id, int position, int *client_fds, int server_fd)
{
    printf("%d\n", position);
    char message[13];
    for (int i = 0; i < position; i++)
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

void handle_create(char *name)
{
    /* function code here */
}

void handle_delete(char *name)
{
    /* function code here */
}

void handle_get(char *dest, char *name)
{
    /* function code here */
}

void handle_st()
{
    fprintf(stdout, "Vizinho externo: %s %s %s\n", server.VE.id, server.VE.ip, server.VE.port);
    fprintf(stdout, "Vizinho Backup: %s %s %s\n", server.VB.id, server.VB.ip, server.VB.port);
}

void handle_sn(char *net)
{
    /* function code here */
}

void handle_sr(char *net)
{
    /* function code here */
}