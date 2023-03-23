#include "handle.h"

extern server_node server;

int handle_join(char *net, char *id)
{
    node_t nodes[MAX_NODES];
    inicialize_nodes(nodes);
    int count = node_list(net, 0, nodes);
    int int_connect, i = 0;
    char id_temp[3] = "";

    if (count > 0)
    {
        while (1)
        {
            int_connect = rand() % count;
            while (verify_node(id, count, nodes) == 0)
            {
                strcpy(id_temp, random_number(id));
                strcpy(id, id_temp);
            }

            if (handle_djoin(net, id, nodes[int_connect].id, nodes[int_connect].ip, nodes[int_connect].port) == 0)
                break;

            strcpy(nodes[int_connect].ip, "0");
            for (i = 0; i <= count; i++)
            {
                if (strcmp(nodes[i].ip, "0") != 0)
                    break;
            }
            if (i == count)
                break;
        }
    }
    if (count == 0 || i == count)
    {
        handle_djoin(net, id, id, server.my_node.ip, server.my_node.port);
    }

    return count;
}

int handle_djoin(char *net, char *id, char *bootid, char *bootIP, char *bootTCP)
{
    char message[50] = "", response[6] = "";
    strcpy(server.my_node.id, id);

    if (strcmp(id, bootid) != 0)
    {
        server.vz[0].fd = tcp_client(bootIP, atoi(bootTCP));
        if (server.vz[0].fd < 0)
            return -1;

        server.vz[0].active = 1;
        sprintf(message, "NEW %s %s %s\n", id, server.my_node.ip, server.my_node.port);
        write(server.vz[0].fd, message, strlen(message));
    }

    strcpy(server.vz[0].id, bootid);
    strcpy(server.vz[0].ip, bootIP);
    strcpy(server.vz[0].port, bootTCP);

    server.vb = server.my_node;

    sprintf(message, "REG %s %s %s %s", net, id, server.my_node.ip, server.my_node.port);
    UDP_server_message(message, response, sizeof(response));
    if (strcmp(response, "OKREG") == 0)
        fprintf(stdout, "node %s is connected to node %s\n", server.my_node.id, server.vz[0].id);
    return 0;
}

void handle_leave(char *net, char *id)
{
    char message[13] = "", response[8] = "";
    for (int i = 0; i < MAX_NODES; i++)
    {
        strcpy(server.names[i], "\0");
        if (server.vz[i].active == 1)
        {
            close(server.vz[i].fd);
            server.vz[i].active = 0;
            strcpy(server.vz[i].id, "\0");
            strcpy(server.vz[i].ip, "\0");
            strcpy(server.vz[i].port, "\0");
            strcpy(server.vb.id, "\0");
            strcpy(server.vb.ip, "\0");
            strcpy(server.vb.port, "\0");
        }
    }

    sprintf(message, "UNREG %s %s", net, id);
    UDP_server_message(message, response, sizeof(response));
    if (strcmp(response, "OKUNREG") == 0)
        fprintf(stdout, "%s left the network %s\n", server.my_node.id, server.net);
}

int handle_create(char *name)
{
    int i, flag = 0;
    for (i = 0; i < MAX_NODES; i++)
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
    for (i = 0; i < MAX_NODES; i++)
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

int handle_get(char *dest, char *name, char *origem)
{

    int flag = 2;
    if (strcmp(dest, server.my_node.id) == 0)
    {
        for (int i = 0; i < MAX_NODES; i++)
        {
            if (strcmp(name, server.names[i]) == 0) // tenho a info
                flag = 1;
        }
        return flag;
    }
    char buff[255] = "";
    int path = 0;
    int dest_int = atoi(dest);

    if (server.exptable[dest_int] != 0) // temos entrada na tabela de espedição
    {
        sprintf(buff, "QUERY %s %s %s \n", dest, origem, name);
        path = server.exptable[dest_int];
        for (int i = 0; i < MAX_NODES; i++)
        {
            if (atoi(server.vz[i].id) == path)
            {
                write(server.vz[i].fd, buff, strlen(buff));
                break;
            }
        }
    }
    else
    {
        sprintf(buff, "QUERY %s %s %s\n", dest, origem, name);
        for (int i = 0; i < MAX_NODES; i++)
        {
            if (server.vz[i].active == 1 && atoi(server.vz[i].id) != atoi(origem))
            {
                write(server.vz[i].fd, buff, strlen(buff));
            }
        }
    }

    return 0;
}

void handle_st()
{
    fprintf(stdout, "Extern: %s %s %s\n", server.vz[0].id, server.vz[0].ip, server.vz[0].port);
    fprintf(stdout, "Backup: %s %s %s\n", server.vb.id, server.vb.ip, server.vb.port);
    fprintf(stdout, "Interns:\n");
    for (int i = 1; i < MAX_NODES; i++)
    {
        if (server.vz[i].active == 1)
            fprintf(stdout, "%s %s %s\n", server.vz[i].id, server.vz[i].ip, server.vz[i].port);
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

void handle_sr()
{
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (server.exptable[i] != 0)
        {
            printf("%d-->%d\n", i, server.exptable[i]);
        }
    }
}

void handle_cr()
{
    for (int i = 0; i < MAX_NODES; i++)
    {
        server.exptable[i] = 0;
    }
}
