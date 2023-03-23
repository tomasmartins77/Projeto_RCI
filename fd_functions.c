#include "fd_functions.h"

extern server_node server;

fd_set handle_menu(fd_set rfds_list, char *ip, char *port)
{
    char message[10] = "", arg1[9] = "", arg2[5] = "", bootid[7] = "", bootIP[16] = "", bootTCP[8] = "", buff[255] = "";
    static int flag_join = 0, flag_create = 0;
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
    else if (strcmp(message, "djoin") == 0 && flag_join == 0)
    {
        strcpy(server.net, arg1);
        handle_djoin(arg1, arg2, bootid, bootIP, bootTCP);

        if (strcmp(arg2, bootid) != 0)
            FD_SET(server.vz[0].fd, &rfds_list);
        flag_join = 1;
    }
    else if (strcmp(message, "djoin") == 0 && flag_join == 1)
        fprintf(stdout, "node already created\n");
    else if (strcmp(message, "leave") == 0 && flag_join == 1)
    {
        flag_join = 0;
        handle_leave(server.net, server.my_node.id);
    }
    else if (strcmp(message, "leave") == 0 && flag_join == 0)
        fprintf(stdout, "no node created\n");
    else if (strcmp(message, "create") == 0 && flag_join == 1)
        flag_create = handle_create(arg1);
    else if (strcmp(message, "create") == 0 && flag_join == 0)
        fprintf(stdout, "no file created");
    else if (strcmp(message, "delete") == 0 && flag_create > 0)
        handle_delete(arg1);
    else if (strcmp(message, "get") == 0 && strcmp(arg1, server.my_node.id) != 0 && flag_join == 1)
        handle_get(arg1, arg2, server.my_node.id);
    else if (strcmp(message, "get") == 0 && strcmp(arg1, server.my_node.id) == 0 && flag_join == 1)
        fprintf(stdout, "cannot get file from yourself\n");
    else if (strcmp(message, "get") == 0 && flag_join == 0)
        fprintf(stdout, "no node created\n");
    else if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag_join == 1)
        handle_st();
    else if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag_join == 0)
        fprintf(stdout, "no node created\n");
    else if ((strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0) && flag_join == 1)
        handle_sn();
    else if ((strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0) && flag_join == 0)
        fprintf(stdout, "no node created\n");
    else if ((strcmp(message, "show routing") == 0 || strcmp(message, "sr") == 0) && flag_join == 1)
        handle_sr(arg2);
    else if ((strcmp(message, "show routing") == 0 || strcmp(message, "sr") == 0) && flag_join == 0)
        fprintf(stdout, "no node created\n");
    else if (strcmp(message, "exit") == 0)
    {
        close(server.my_node.fd);
        fprintf(stdout, "exiting program\n");
        exit(1);
    }
    else if (strcmp(message, "clear") == 0)
        clear(arg1);
    else if (strcmp(message, "clear route") == 0 || strcmp(message, "cr") == 0)
        handle_cr();
    else
        fprintf(stdout, "invalid command\n");
    return rfds_list;
}

fd_set client_fd_set(fd_set rfds_list, int x)
{
    char buff[1024] = "", str_temp[10] = "", message[50] = "", origin[3] = "", dest[3] = "", content[100] = "";
    node_t temp;
    memset(buff, 0, 1024);
    int save = server.vz[x].fd;
    int intr = 0, i;

    for (intr = 1; intr < MAX_NODES; intr++)
    {
        if (server.vz[intr].active == 1)
            break;
    }

    if (read(server.vz[x].fd, buff, 1024) == 0)
    {
        fprintf(stdout, "%s has left network %s\n", server.vz[x].id, server.net);
        withdraw(atoi(server.vz[x].id));

        close(server.vz[x].fd);
        if (x > 0)
        {
            server.vz[x].active = 0;
        }
        else if (strcmp(server.my_node.id, server.vb.id) != 0) // VE saiu e nao é ancora, tem VI
        {
            server.vz[x] = server.vb;
            server.vz[x].fd = tcp_client(server.vb.ip, atoi(server.vb.port));
            fprintf(stdout, "node %s is connected to node %s\n", server.my_node.id, server.vz[x].id);

            sprintf(message, "NEW %s %s %s\n", server.my_node.id, server.my_node.ip, server.my_node.port);
            write(server.vz[x].fd, message, strlen(message));

            sprintf(message, "EXTERN %s %s %s\n", server.vz[x].id, server.vz[x].ip, server.vz[x].port);
            // for loop a enviar EXTERN aos intr
            for (i = 1; i < MAX_NODES; i++)
            {
                if (server.vz[i].active == 1)
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
                if (server.vz[i].active == 1)
                {
                    write(server.vz[i].fd, message, strlen(message));
                }
            }
            server.vz[intr].fd = 0;
        }
        else
        {
            server.vz[x] = server.my_node;
            server.vz[x].active = 0;
        }
    }
    else
    {
        sscanf(buff, "%s", str_temp);
        if (strcmp(str_temp, "NEW") == 0)
        {
            sscanf(buff, "%s %s %s %s\n", str_temp, temp.id, temp.ip, temp.port);
            server.vz[x] = temp;
            server.vz[x].fd = save;
            sprintf(buff, "EXTERN %s %s %s\n", server.vz[0].id, server.vz[0].ip, server.vz[0].port);
            write(server.vz[x].fd, buff, strlen(buff));
            server.exptable[atoi(temp.id)] = atoi(temp.id);
            fprintf(stdout, "node %s is connected to node %s\n", server.vz[x].id, server.my_node.id);
        }
        if (strcmp(str_temp, "EXTERN") == 0)
        {
            sscanf(buff, "%s %s %s %s\n", str_temp, temp.id, temp.ip, temp.port);
            server.vb = temp;
            server.exptable[atoi(server.vz[x].id)] = atoi(server.vz[x].id);
            server.exptable[atoi(temp.id)] = atoi(server.vz[x].id);
        }
        if (strcmp(str_temp, "QUERY") == 0)
        {
            sscanf(buff, "%s %s %s %s\n", str_temp, dest, origin, content);
            server.exptable[atoi(origin)] = atoi(server.vz[x].id);
            int res = handle_get(dest, content, origin);
            if (res == 1)
            {
                sprintf(buff, "CONTENT %s %s %s\n", origin, dest, content);
                write(server.vz[x].fd, buff, strlen(buff));
            }
            if (res == 2)
            {
                sprintf(buff, "NOCONTENT %s %s %s\n", origin, dest, content);
                write(server.vz[x].fd, buff, strlen(buff));
            }
        }
        if (strcmp(str_temp, "NOCONTENT") == 0)
        {
            sscanf(buff, "%s %s %s %s\n", str_temp, origin, dest, content);
            server.exptable[atoi(temp.ip)] = atoi(server.vz[x].id);
            if (strcmp(temp.id, server.my_node.id) != 0)
            {
                int temp_ip = server.exptable[atoi(temp.id)];
                for (int i = 0; i < MAX_NODES; i++)
                {
                    if (atoi(server.vz[i].id) == temp_ip)
                    {
                        sprintf(buff, "NOCONTENT %s %s %s\n", origin, dest, content);
                        write(server.vz[i].fd, buff, strlen(buff));
                    }
                }
            }
            else
                fprintf(stdout, "name not available\n");
        }
        if (strcmp(str_temp, "CONTENT") == 0)
        {
            sscanf(buff, "%s %s %s %s\n", str_temp, origin, dest, content);
            server.exptable[atoi(temp.ip)] = atoi(server.vz[x].id);
            if (strcmp(temp.id, server.my_node.id) != 0)
            {
                int temp_ip = server.exptable[atoi(temp.id)];
                for (int i = 0; i < MAX_NODES; i++)
                {
                    if (atoi(server.vz[i].id) == temp_ip)
                    {
                        sprintf(buff, "CONTENT %s %s %s\n", origin, dest, content);
                        write(server.vz[i].fd, buff, strlen(buff));
                    }
                }
            }
            else
                fprintf(stdout, "name is available\n");
        }
        if (strcmp(str_temp, "WITHDRAW") == 0)
        {
            sscanf(buff, "%s %s\n", str_temp, dest);
            withdraw(atoi(dest));
        }
    }

    return rfds_list;
}

void withdraw(int x)
{
    char buff[100] = "";
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