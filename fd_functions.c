#include "fd_functions.h"

extern server_node server;

fd_set handle_menu(fd_set rfds_list, char *ip, char *port)
{
    char buff[1024], str_temp[10], id_temp[3], ip_temp[16], port_temp[6];
    char message[10], arg1[9], arg2[5], bootid[7], bootIP[7], bootTCP[8];
    static int flag_join = 0, flag_delete = 0, flag_create = 0;
    int count = 0;

    fgets(buff, 255, stdin); // LE o que ta escrito
    sscanf(buff, "%s %s %s %s %s %s", message, arg1, arg2, bootid, bootIP, bootTCP);
    if (strcmp(message, "join") == 0 && flag_join == 0)
    {
        strcpy(server.net, arg1);
        count = handle_join(arg1, arg2);

        if (count > 0)
            FD_SET(server.vz[0].fd, &rfds_list);
        flag_join = 1;
    }
    else if (strcmp(message, "join") == 0 && flag_join == 1)
        fprintf(stdout, "node already created\n");
    if (strcmp(message, "djoin") == 0 && flag_join == 0)
    {
        strcpy(server.net, arg1);
        handle_djoin(arg1, arg2, bootid, bootIP, bootTCP);

        if (count > 0)
            FD_SET(server.vz[0].fd, &rfds_list);
        flag_join = 1;
    }
    else if (strcmp(message, "djoin") == 0 && flag_join == 1)
        fprintf(stdout, "node already created\n");
    if (strcmp(message, "leave") == 0 && flag_join == 1)
    {
        flag_join = 0;
        handle_leave(server.net, server.my_node.id);
    }
    else if (strcmp(message, "leave") == 0 && flag_join == 0)
        fprintf(stdout, "no node created\n");
    if (strcmp(message, "create") == 0 && flag_join == 1)
        flag_create = handle_create(arg1);
    else if (strcmp(message, "create") == 0 && flag_join == 0)
        fprintf(stdout, "no file created");
    if (strcmp(message, "delete") == 0 && flag_create > 0)
        handle_delete(arg1);
    if (strcmp(message, "get") == 0)
        handle_get(arg1, arg2, server.my_node.id);
    if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag_join == 1)
        handle_st();
    else if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag_join == 0)
        fprintf(stdout, "no node created\n");
    if (strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0 && flag_join == 1)
        handle_sn();
    else if (strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0 && flag_join == 0)
        fprintf(stdout, "no node created\n");
    if (strcmp(message, "show routing") == 0 || strcmp(message, "sr") == 0 && flag_join == 1)
        handle_sr(arg2);
    else if (strcmp(message, "show routing") == 0 || strcmp(message, "sr") == 0 && flag_join == 0)
        fprintf(stdout, "no node created\n");
    if (strcmp(message, "exit") == 0)
    {
        close(server.my_node.fd);
        fprintf(stdout, "exiting program\n");
        exit(1);
    }
    if (strcmp(message, "clear") == 0)
    {
        clear(arg1);
    }
    return rfds_list;
}

fd_set client_fd_set(fd_set rfds_list, int x)
{
    char buff[1024] = "", str_temp[10];
    char message[50], response[6];
    node_t temp;
    memset(buff, 0, 1024);
    int save = server.vz[x].fd;
    int intr = 0, i;

    for (intr = 1; intr < MAX_NODES; intr++)
    {
        if (server.vz[intr].fd != -1)
            break;
    }

    if (read(server.vz[x].fd, buff, 1024) == 0)
    {
        printf("%s\n", server.vz[x].id);
        withdraw(atoi(server.vz[x].id));

        close(server.vz[x].fd);
        if (x > 0)
        {
            server.vz[x].fd = -1;
        }
        else if (strcmp(server.my_node.id, server.vb.id) != 0) // VE saiu e nao é ancora, tem VI
        {
            server.vz[x] = server.vb;
            server.vz[x].fd = tcp_client(server.vb.ip, atoi(server.vb.port));

            sprintf(message, "NEW %s %s %s\n", server.my_node.id, server.my_node.ip, server.my_node.port);
            write(server.vz[x].fd, message, strlen(message));

            sprintf(message, "EXTERN %s %s %s\n", server.vz[x].id, server.vz[x].ip, server.vz[x].port);
            // for loop a enviar EXTERN aos intr
            for (i = 1; i < MAX_NODES; i++)
            {
                if (server.vz[i].fd != -1)
                {
                    write(server.vz[i].fd, message, strlen(message));
                }
            }
        }
        else if (intr != MAX_NODES)
        {
            server.vz[x] = server.vz[intr];

            sprintf(message, "EXTERN %s %s %s\n", server.vz[x].id, server.vz[x].ip, server.vz[x].port);
            for (i = 1; i < MAX_NODES; i++)
            {
                if (server.vz[i].fd != -1)
                {
                    write(server.vz[i].fd, message, strlen(message));
                }
            }
            server.vz[intr].fd = -1;
        }
        else
        {
            server.vz[x] = server.my_node;
            server.vz[x].fd = -1;
        }
    }
    else
    {
        sscanf(buff, "%s %s %s %s", str_temp, temp.id, temp.ip, temp.port);
        if (strcmp(str_temp, "NEW") == 0)
        {
            server.vz[x] = temp;
            server.vz[x].fd = save;
            sprintf(buff, "EXTERN %s %s %s\n", server.vz[0].id, server.vz[0].ip, server.vz[0].port);
            write(server.vz[x].fd, buff, strlen(buff));
            server.exptable[atoi(temp.id)] = atoi(temp.id);
        }
        if (strcmp(str_temp, "EXTERN") == 0)
        {
            server.vb = temp;
            server.exptable[atoi(server.vz[x].id)] = atoi(server.vz[x].id);
            server.exptable[atoi(temp.id)] = atoi(server.vz[x].id);
        }
        if (strcmp(str_temp, "QUERY") == 0)
        {
            server.exptable[atoi(temp.ip)] = atoi(server.vz[x].id);
            int res = handle_get(temp.id, temp.port, temp.ip);
            if (res == 1)
            {
                sprintf(buff, "CONTENT %s %s %s\n", temp.ip, temp.id, temp.port);
                write(server.vz[x].fd, buff, strlen(buff));
            }
            if (res == 2)
            {
                sprintf(buff, "NOCONTENT %s %s %s\n", temp.ip, temp.id, temp.port);
                write(server.vz[x].fd, buff, strlen(buff));
            }
        }
        if (strcmp(str_temp, "NOCONTENT") == 0)
        {
            server.exptable[atoi(temp.ip)] = atoi(server.vz[x].id);
            if (strcmp(temp.id, server.my_node.id) != 0)
            {
                int temp_ip = server.exptable[atoi(temp.id)];
                for (int i = 0; i < 99; i++)
                {
                    if (atoi(server.vz[i].id) == temp_ip)
                    {
                        sprintf(buff, "NOCONTENT %s %s %s\n", temp.id, temp.ip, temp.port);
                        write(server.vz[i].fd, buff, strlen(buff));
                    }
                }
            }
        }
        if (strcmp(str_temp, "CONTENT") == 0)
        {
            server.exptable[atoi(temp.ip)] = atoi(server.vz[x].id);
            if (strcmp(temp.id, server.my_node.id) != 0)
            {
                int temp_ip = server.exptable[atoi(temp.id)];
                for (int i = 0; i < 99; i++)
                {
                    if (atoi(server.vz[i].id) == temp_ip)
                    {
                        sprintf(buff, "CONTENT %s %s %s\n", temp.id, temp.ip, temp.port);
                        write(server.vz[i].fd, buff, strlen(buff));
                    }
                }
            }
        }
        if (strcmp(str_temp, "WITHDRAW") == 0)
        {
            withdraw(atoi(temp.id));
        }
    }

    return rfds_list;
}

void withdraw(int x)
{
    char buff[100];
    char x_c[3] = "";
    sprintf(x_c, "%02d", x);
    server.exptable[x] = 0;
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (server.exptable[i] == x)
            server.exptable[i] = 0;
        if (i != x)
        {
            sprintf(buff, "WITHDRAW %s\n", x_c);
            write(server.vz[i].fd, buff, strlen(buff));
        }
    }
}