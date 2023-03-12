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
    fprintf(stdout, "UNREG %s %s\n", net, id);
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

fd_set handle_menu(int *position, fd_set rfds_list, int *client_fds, int server_fd, char *ip, char *port, int intr)
{
    char buff[1024], str_temp[10], id_temp[3], ip_temp[16], port_temp[6];
    char message[10], arg1[9], arg2[5], bootid[7], bootIP[7], bootTCP[8];
    static int flag_join = 0, flag_delete = 0, flag_create = 0;
    static char net[4];
    int count = 0;

    fgets(buff, 255, stdin); // LE o que ta escrito
    sscanf(buff, "%s %s %s %s %s %s", message, arg1, arg2, bootid, bootIP, bootTCP);
    if (strcmp(message, "join") == 0 && flag_join == 0)
    {
        count = handle_join(arg1, arg2, ip, port, (*position), client_fds);
        strcpy(net, arg1);
        if (count > 0)
            FD_SET(client_fds[(*position)++], &rfds_list);
        flag_join = 1;
    }
    else if (strcmp(message, "join") == 0 && flag_join == 1)
        fprintf(stdout, "node already created\n");
    if (strcmp(message, "leave") == 0 && flag_join == 1)
    {
        flag_join = 0;
        handle_leave(net, server.my_node.id, (*position), client_fds);
    }
    else if (strcmp(message, "leave") == 0 && flag_join == 0)
        fprintf(stdout, "no node created\n");
    if (strcmp(message, "djoin") == 0)
        handle_djoin(arg1, arg2, bootid, bootIP, bootTCP);
    if (strcmp(message, "create") == 0 && flag_join == 1)
        flag_create = handle_create(arg1);
    else if (strcmp(message, "create") == 0 && flag_join == 0)
        fprintf(stdout, "no file created");
    if (strcmp(message, "delete") == 0 && flag_create > 0)
        handle_delete(arg1);
    if (strcmp(message, "get") == 0)
        handle_get(arg1, arg2);
    if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag_join == 1)
        handle_st(intr);
    else if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag_join == 0)
        fprintf(stdout, "no node created\n");
    if (strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0)
        handle_sn();
    if (strcmp(message, "show routing") == 0 || strcmp(message, "sr") == 0)
        handle_sr(arg2);
    if (strcmp(message, "exit") == 0)
    {
        close(server_fd);
        FD_CLR(server_fd, &rfds_list);
        fprintf(stdout, "exiting program\n");
        exit(1);
    }
    if (strcmp(message, "clear") == 0)
    {
        clear(arg1);
        exit(1);
    }
    return rfds_list;
}

fd_set client_fd_set(fd_set rfds_list, int *client_fds, int x, int *intr)
{
    char buff[1024], str_temp[10];
    node_t temp;
    memset(buff, 0, 1024);

    int bytes_received = read(client_fds[x], buff, 1024);
    if (bytes_received == 0)
    {
        printf("asdasdasd\n");
        // else if x pertence a intr -> intr = intr - x
        if (strcmp(server.my_node.id, server.VB.id) != 0)
        {
            server.VE = server.VB;
            sprintf(buffer, "NEW %s %s %s\n", server.my_node.id, server.my_node.ip, server.my_node.port);
            write(client_fds[x], buff, 1024);
            sprintf(buff, "EXTERN %s %s %s\n", server.VE.id, server.VE.ip, server.VE.port);
            // for loop a enviar EXTERN aos intr
            for (int i = 0; i < (*intr); i++)
            {
            }
        }
        // else if intr != NULL -> escolhe intr random e EX = intr random
        //  envia EXTERN para intr     intr = intr - intr random
        else
        {
            server.VE = server.my_node;
        }
        FD_CLR(client_fds[x], &rfds_list);
    }
    fprintf(stdout, "%s", buff);
    sscanf(buff, "%s %s %s %s", str_temp, temp.id, temp.ip, temp.port);

    if (strcmp(str_temp, "NEW") == 0)
    {
        if (strcmp(server.my_node.id, server.VE.id) == 0) // ancora
        {
            server.VE = temp;
        }
        else
        {
            server.VI[(*intr)++] = temp;
        }
        sprintf(buff, "EXTERN %s %s %s\n", server.VE.id, server.VE.ip, server.VE.port);
        write(client_fds[x], buff, 1024);
    }
    if (strcmp(str_temp, "EXTERN") == 0)
    {
        server.VB = temp;
    }
    return rfds_list;
}