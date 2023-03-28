#include "fd_functions.h"

extern server_node server;

fd_set handle_menu(fd_set rfds_list, char *ip, char *port, char *connect_ip, char *connect_port)
{
    char message[10] = "", arg1[9] = "", arg2[MAX_NAME] = "", bootid[7] = "", bootIP[16] = "", bootTCP[8] = "", buff[MAX_BUFFER] = "";
    static int flag_join = 0, flag_create = 0, flag_djoin = 0;
    int count = 0;

    fgets(buff, MAX_BUFFER, stdin); // LE o que ta escrito
    sscanf(buff, "%s %s %s %s %s %s", message, arg1, arg2, bootid, bootIP, bootTCP);
    if (strcmp(message, "join") == 0 && flag_join == 0 && flag_djoin == 0)
    {
        if (check_input_format(buff, message) == 0)
        {
            fprintf(stdout, "Invalid arguments\n");
            return rfds_list;
        }
        flag_join = server_creation();

        strcpy(server.net, arg1);

        count = handle_join(arg1, arg2, connect_ip, connect_port);

        if (count > 0)
            FD_SET(server.vz[0].fd, &rfds_list);
    }
    else if (strcmp(message, "join") == 0 && flag_join == 1 && flag_djoin == 0)
        fprintf(stdout, "node already created\n");
    else if (strcmp(message, "djoin") == 0 && flag_djoin == 0 && flag_join == 0)
    {
        if (check_input_format(buff, message) == 0)
        {
            fprintf(stdout, "Invalid arguments\n");
            return rfds_list;
        }
        flag_djoin = server_creation();
        strcpy(server.net, arg1);

        handle_djoin(arg1, arg2, bootid, bootIP, bootTCP, connect_ip, connect_port);

        if (strcmp(arg2, bootid) != 0)
            FD_SET(server.vz[0].fd, &rfds_list);
        flag_join = 1;
    }
    else if (strcmp(message, "djoin") == 0 && flag_djoin == 1 && flag_join == 0)
        fprintf(stdout, "node already created\n");
    else if (strcmp(message, "leave") == 0 && (flag_join == 1 || flag_djoin == 1))
    {
        if (flag_join == 1)
            handle_leave(server.net, server.my_node.id, connect_ip, connect_port, 1);
        else if (flag_djoin == 1)
            handle_leave(server.net, server.my_node.id, server.vb.ip, server.vb.port, 0);

        flag_join = 0;
        flag_djoin = 0;
    }
    else if (strcmp(message, "leave") == 0 && (flag_join == 0 || flag_djoin == 0))
        fprintf(stdout, "no node created\n");
    else if (strcmp(message, "create") == 0 && (flag_join == 1 || flag_djoin == 1))
        flag_create = handle_create(arg1, flag_create);
    else if (strcmp(message, "create") == 0 && (flag_join == 0 || flag_djoin == 0))
        fprintf(stdout, "no file created\n");
    else if (strcmp(message, "delete") == 0 && flag_create > 0 && (flag_join == 0 || flag_djoin == 0))
    {
        handle_delete(arg1);
        flag_create--;
    }
    else if (strcmp(message, "delete") == 0 && flag_create == 0 && (flag_join == 0 || flag_djoin == 0))
        fprintf(stdout, "no file to be deleted\n");
    else if (strcmp(message, "get") == 0 && strcmp(arg1, server.my_node.id) != 0 && (flag_join == 1 || flag_djoin == 1))
    {
        if (check_input_format(buff, message) == 0)
        {
            fprintf(stdout, "Invalid arguments\n");
            return rfds_list;
        }
        handle_get(arg1, arg2, server.my_node.id, -1);
    }
    else if (strcmp(message, "get") == 0 && strcmp(arg1, server.my_node.id) == 0 && (flag_join == 1 || flag_djoin == 1))
        fprintf(stdout, "cannot get file from yourself\n");
    else if (strcmp(message, "get") == 0 && flag_join == 0)
        fprintf(stdout, "no node created\n");
    else if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && (flag_join == 1 || flag_djoin == 1))
        handle_st();
    else if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && (flag_join == 0 || flag_djoin == 0))
        fprintf(stdout, "no node created\n");
    else if ((strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0) && (flag_join == 1 || flag_djoin == 1))
        handle_sn();
    else if ((strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0) && (flag_join == 0 || flag_djoin == 0))
        fprintf(stdout, "no node created\n");
    else if ((strcmp(message, "show routing") == 0 || strcmp(message, "sr") == 0) && (flag_join == 1 || flag_djoin == 1))
        handle_sr(arg2);
    else if ((strcmp(message, "show routing") == 0 || strcmp(message, "sr") == 0) && (flag_join == 0 || flag_djoin == 0))
        fprintf(stdout, "no node created\n");
    else if (strcmp(message, "exit") == 0)
    {
        if (flag_join == 1)
            handle_leave(server.net, server.my_node.id, connect_ip, connect_port, 1);
        else if (flag_djoin == 1)
            handle_leave(server.net, server.my_node.id, server.vb.ip, server.vb.port, 0);

        close(server.my_node.fd);
        fprintf(stdout, "exiting program\n");
        exit(1);
    }
    else if (strcmp(message, "clear") == 0)
        clear(arg1, connect_ip, connect_port);
    else if ((strcmp(message, "clear route") == 0 || strcmp(message, "cr") == 0) && (flag_join == 1 || flag_djoin == 1))
        handle_cr();
    else if ((strcmp(message, "clear route") == 0 || strcmp(message, "cr") == 0) && (flag_join == 0 || flag_djoin == 0))
        fprintf(stdout, "no node created\n");
    else if (strcmp(message, "show") == 0)
        show(arg1, connect_ip, connect_port);
    else
        fprintf(stdout, "invalid command\n");
    return rfds_list;
}

fd_set client_fd_set(fd_set rfds_list, int x)
{
    char str_temp[10] = "", origin[3] = "", dest[3] = "", content[100] = "", *ptr = NULL, buffer[MAX_BUFFER] = "";
    node_t temp = {};
    int save = server.vz[x].fd;
    int num_bytes = 0;

    ptr = buffer;

    while (strcmp(ptr - 1, "\n") != 0)
    {
        num_bytes = read(server.vz[x].fd, ptr, MAX_BUFFER);
        if (num_bytes == -1)
        {
            fprintf(stdout, "error reading from socket\n");
            return rfds_list;
        }
        if (num_bytes == 0)
        {
            leave(x);
            return rfds_list;
        }
        ptr += num_bytes;
    }

    sscanf(buffer, "%s", str_temp);

    if (strcmp(str_temp, "NEW") == 0)
    {
        sscanf(buffer, "%s %s %s %s\n", str_temp, temp.id, temp.ip, temp.port);
        server.vz[x] = temp;
        server.vz[x].fd = save;
        server.vz[x].active = 1;
        sprintf(buffer, "EXTERN %s %s %s\n", server.vz[0].id, server.vz[0].ip, server.vz[0].port);
        write(server.vz[x].fd, buffer, strlen(buffer));

        fprintf(stdout, "node %s is connected to node %s\n", server.vz[x].id, server.my_node.id);
    }
    else if (strcmp(str_temp, "EXTERN") == 0)
    {
        sscanf(buffer, "%s %s %s %s\n", str_temp, temp.id, temp.ip, temp.port);
        server.vb = temp;
        if (strcmp(server.vz[x].id, server.vb.id) == 0)
            leave(x);
        else
            fprintf(stdout, "my backup is node %s\n", server.vb.id);
    }
    else if (strcmp(str_temp, "QUERY") == 0)
    {
        sscanf(buffer, "%s %s %s %s\n", str_temp, dest, origin, content);
        server.exptable[atoi(origin)] = atoi(server.vz[x].id);
        int res = handle_get(dest, content, origin, x);
        if (res == 1)
        {
            sprintf(buffer, "CONTENT %s %s %s\n", origin, dest, content);
            write(server.vz[x].fd, buffer, strlen(buffer));
        }
        if (res == 2)
        {
            sprintf(buffer, "NOCONTENT %s %s %s\n", origin, dest, content);
            write(server.vz[x].fd, buffer, strlen(buffer));
        }
    }
    else if (strcmp(str_temp, "NOCONTENT") == 0)
    {
        sscanf(buffer, "%s %s %s %s\n", str_temp, origin, dest, content);
        server.exptable[atoi(dest)] = atoi(server.vz[x].id);
        if (strcmp(origin, server.my_node.id) != 0)
        {
            int temp_id = server.exptable[atoi(origin)];
            for (int i = 0; i < MAX_NODES; i++)
            {
                if (atoi(server.vz[i].id) == temp_id)
                {
                    sprintf(buffer, "NOCONTENT %s %s %s\n", origin, dest, content);
                    write(server.vz[i].fd, buffer, strlen(buffer));
                }
            }
        }
        else
            fprintf(stdout, "name: %s not available\n", content);
    }
    else if (strcmp(str_temp, "CONTENT") == 0)
    {
        sscanf(buffer, "%s %s %s %s\n", str_temp, origin, dest, content);
        server.exptable[atoi(dest)] = atoi(server.vz[x].id);
        if (strcmp(origin, server.my_node.id) != 0)
        {
            int temp_id = server.exptable[atoi(origin)];
            for (int i = 0; i < MAX_NODES; i++)
            {
                if (atoi(server.vz[i].id) == temp_id)
                {
                    sprintf(buffer, "CONTENT %s %s %s\n", origin, dest, content);
                    write(server.vz[i].fd, buffer, strlen(buffer));
                }
            }
        }
        else
            fprintf(stdout, "name: %s is available\n", content);
    }
    else if (strcmp(str_temp, "WITHDRAW") == 0)
    {
        sscanf(buffer, "%s %s\n", str_temp, dest);
        withdraw(atoi(dest), x);
    }
    else
        leave(x);

    return rfds_list;
}

void withdraw(int node_le, int x)
{
    char buff[100] = "";
    char char_node_le[3] = "";
    sprintf(char_node_le, "%02d", node_le);
    server.exptable[node_le] = -1;

    for (int i = 0; i < MAX_NODES; i++)
    {
        if (server.exptable[i] == node_le)
        {
            server.exptable[i] = -1;
        }
        if (server.vz[i].active == 1 && i != x)
        {
            sprintf(buff, "WITHDRAW %s\n", char_node_le);
            write(server.vz[i].fd, buff, strlen(buff));
        }
    }
}

void leave(int x)
{
    int intr = 0, i;
    char message[MAX_BUFFER] = "";

    for (intr = 1; intr < MAX_NODES; intr++)
    {
        if (server.vz[intr].active == 1)
            break;
    }
    fprintf(stdout, "%s has left network %s\n", server.vz[x].id, server.net);
    withdraw(atoi(server.vz[x].id), x);

    close(server.vz[x].fd);

    if (x > 0)
        server.vz[x].active = 0;
    else if (strcmp(server.my_node.id, server.vb.id) != 0) // VE saiu e nao é ancora, tem VI
    {
        server.vz[x] = server.vb;
        server.vz[x].active = 1;
        server.vz[x].fd = tcp_client(server.vb.ip, atoi(server.vb.port));
        fprintf(stdout, "node %s is now connected to node %s\n", server.my_node.id, server.vz[x].id);

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
        server.vz[x].active = 1;

        sprintf(message, "EXTERN %s %s %s\n", server.vz[x].id, server.vz[x].ip, server.vz[x].port);
        for (i = 1; i < MAX_NODES; i++)
        {
            if (server.vz[i].active == 1)
            {
                write(server.vz[i].fd, message, strlen(message));
            }
        }
        fprintf(stdout, "choosing node %s as new anchor\n", server.vz[x].id);
        server.vz[intr].active = 0;
    }
    else
    {
        fprintf(stdout, "node %s is alone in network %s\n", server.my_node.id, server.net);
        server.vz[x] = server.my_node;
        server.vz[x].active = 0;
    }
}