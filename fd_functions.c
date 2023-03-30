#include "fd_functions.h"

extern server_node server;

/*
 * Function: handle_menu
 * Brief:
 *   Function that handles the menu options.
 * Parameters:
 *   rfds_list: list of file descriptors
 *   ip: IP address of the server
 *   port: port number of the server
 *   connect_ip: IP address of the server
 *   connect_port: port number of the server
 * Return Value:
 *   rfds_list: list of file descriptors
 */
fd_set handle_menu(fd_set rfds_list, char *ip, char *port, char *connect_ip, char *connect_port)
{
    char message[10] = "", arg1[9] = "", arg2[MAX_NAME] = "", bootid[7] = "", bootIP[16] = "", bootTCP[8] = "", buff[MAX_BUFFER] = "";
    static int flag_join = 0, flag_create = 0, flag_djoin = 0;
    int count = 0;

    if (fgets(buff, MAX_BUFFER, stdin) == NULL) // LE o que ta escrito
    {
        fprintf(stdout, "error reading from stdin\n");
        return rfds_list;
    }
    sscanf(buff, "%s %s %s %s %s %s", message, arg1, arg2, bootid, bootIP, bootTCP);
    if (strcmp(message, "join") == 0 && flag_join == 0 && flag_djoin == 0)
    {
        if (check_input_format(buff, message) == 0)
        {
            fprintf(stdout, "Invalid arguments\n");
            return rfds_list;
        }
        strcpy(server.net, arg1);

        count = handle_join(arg1, arg2, connect_ip, connect_port);

        if (count > 0 && server.vz[0].active == 1)
            FD_SET(server.vz[0].fd, &rfds_list);
        flag_join = 1;
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
        strcpy(server.net, arg1);

        handle_djoin(arg1, arg2, bootid, bootIP, bootTCP, connect_ip, connect_port);

        if (strcmp(arg2, bootid) != 0)
        {
            FD_SET(server.vz[0].fd, &rfds_list);
            fprintf(stdout, "node %s is connected to node %s\n", server.my_node.id, server.vz[0].id);
        }
        flag_djoin = 1;
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
    else if (strcmp(message, "clr") == 0)
    {
        if (system("clear") == -1)
            fprintf(stdout, "error\n");
        fprintf(stdout, "id: %s   IP: %s   PORT: %s\n", server.my_node.id, server.my_node.ip, server.my_node.port);
    }
    else
        fprintf(stdout, "invalid command\n");

    return rfds_list;
}

/*
 * This function is used to read from a client socket and handle the messages received
 * parameters:
 *     rfds_list - the list of file descriptors
 *     x - the index of the client in the vector
 * return value:
 *    rfds_list - the list of file descriptors
 */
fd_set client_fd_set(fd_set rfds_list, int x)
{
    char str_temp[10] = "", origin[3] = "", dest[3] = "", content[100] = "", *token, message[MAX_BUFFER] = "", aux[MAX_BUFFER] = "";
    node_t temp = {};
    int num_bytes = 0, len = 0;

    num_bytes = read(server.vz[x].fd, server.vz[x].buffer + server.vz[x].bytes_received, MAX_BUFFER - server.vz[x].bytes_received);
    if (num_bytes == -1)
    {
        fprintf(stdout, "error reading from socket\n");
        memset(server.vz[x].buffer, 0, MAX_BUFFER);
        server.vz[x].bytes_received = 0;
        leave(x);
        return rfds_list;
    }
    if (num_bytes == 0)
    {
        leave(x);
        return rfds_list;
    }

    strcpy(aux, server.vz[x].buffer);

    int i, save = 0;
    for (i = 0; i < strlen(aux); i++)
    {
        if (aux[i] == '\n')
            save = i;
    }
    aux[save + 1] = '\0';

    server.vz[x].bytes_received += num_bytes;

    if (strchr(aux, '\n') != NULL)
    {
        token = strtok(aux, "\n");

        len = strlen(token);
        while (token != NULL)
        {
            sscanf(token, "%s", str_temp);

            if (strcmp(str_temp, "NEW") == 0)
            {
                sscanf(token, "%s %s %s %s\n", str_temp, temp.id, temp.ip, temp.port);

                strcpy(server.vz[x].id, temp.id);
                strcpy(server.vz[x].ip, temp.ip);
                strcpy(server.vz[x].port, temp.port);
                server.vz[x].active = 1;

                sprintf(message, "EXTERN %s %s %s\n", server.vz[0].id, server.vz[0].ip, server.vz[0].port);
                if (write(server.vz[x].fd, message, strlen(message)) == -1)
                {
                    fprintf(stdout, "error writing to socket\n");
                    return rfds_list;
                }

                fprintf(stdout, "node %s is connected to node %s\n", server.vz[x].id, server.my_node.id);
            }
            else if (strcmp(str_temp, "EXTERN") == 0)
            {
                sscanf(token, "%s %s %s %s\n", str_temp, temp.id, temp.ip, temp.port);

                server.vb = temp;

                if (strcmp(server.vz[x].id, server.vb.id) == 0)
                    leave(x);
                else
                    fprintf(stdout, "my backup is node %s\n", server.vb.id);
            }
            else if (strcmp(str_temp, "QUERY") == 0)
            {
                sscanf(token, "%s %s %s %s\n", str_temp, dest, origin, content);

                server.exptable[atoi(origin)] = atoi(server.vz[x].id);
                int res = handle_get(dest, content, origin, x);
                if (res == 1)
                {
                    sprintf(message, "CONTENT %s %s %s\n", origin, dest, content);
                    if (write(server.vz[x].fd, message, strlen(message)) == -1)
                    {
                        fprintf(stdout, "error writing to socket\n");
                        return rfds_list;
                    }
                }
                if (res == 2)
                {
                    sprintf(message, "NOCONTENT %s %s %s\n", origin, dest, content);
                    if (write(server.vz[x].fd, message, strlen(message)) == -1)
                    {
                        fprintf(stdout, "error writing to socket\n");
                        return rfds_list;
                    }
                }
            }
            else if (strcmp(str_temp, "NOCONTENT") == 0)
            {
                sscanf(token, "%s %s %s %s\n", str_temp, origin, dest, content);

                server.exptable[atoi(dest)] = atoi(server.vz[x].id);
                if (strcmp(origin, server.my_node.id) != 0)
                {
                    int temp_id = server.exptable[atoi(origin)];
                    for (int i = 0; i < MAX_NODES; i++)
                    {
                        if (atoi(server.vz[i].id) == temp_id)
                        {
                            sprintf(message, "NOCONTENT %s %s %s\n", origin, dest, content);
                            if (write(server.vz[i].fd, message, strlen(message)) == -1)
                            {
                                fprintf(stdout, "error writing to socket\n");
                                return rfds_list;
                            }
                        }
                    }
                }
                else
                    fprintf(stdout, "name: %s not available\n", content);
            }
            else if (strcmp(str_temp, "CONTENT") == 0)
            {
                sscanf(token, "%s %s %s %s\n", str_temp, origin, dest, content);

                server.exptable[atoi(dest)] = atoi(server.vz[x].id);
                if (strcmp(origin, server.my_node.id) != 0)
                {
                    int temp_id = server.exptable[atoi(origin)];
                    for (int i = 0; i < MAX_NODES; i++)
                    {
                        if (atoi(server.vz[i].id) == temp_id)
                        {
                            sprintf(message, "CONTENT %s %s %s\n", origin, dest, content);
                            if (write(server.vz[i].fd, message, strlen(message)) == -1)
                            {
                                fprintf(stdout, "error writing to socket\n");
                                return rfds_list;
                            }
                        }
                    }
                }
                else
                    fprintf(stdout, "name: %s is available\n", content);
            }
            else if (strcmp(str_temp, "WITHDRAW") == 0)
            {
                sscanf(token, "%s %s\n", str_temp, dest);
                withdraw(atoi(dest), x);
            }
            else
            {
                fprintf(stdout, "error: %s\n", server.vz[x].id);
                memset(server.vz[x].buffer, 0, MAX_BUFFER);
                server.vz[x].bytes_received = 0;

                leave(x);
                break;
            }

            token = strtok(NULL, "\n");

            memmove(server.vz[x].buffer, server.vz[x].buffer + len + 1, server.vz[x].bytes_received);
            server.vz[x].bytes_received -= len + 1;
        }
    }
    return rfds_list;
}

/*
 * This function is used to send withdraw messages to all nodes necessary
 * when a node leaves the network.
 * Parameters:
 *      node_le: the node that is leaving the network
 *      x: the index of the node that is leaving the network
 */
void withdraw(int node_le, int x)
{
    char buff[MAX_BUFFER] = "";
    char char_node_le[3] = "";

    // convert node_le to a 2-digit string
    sprintf(char_node_le, "%02d", node_le);

    // mark node_le as withdrawn in the server's expadition table
    server.exptable[node_le] = -1;

    // iterate through exptable to remove node_le from all entries
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (server.exptable[i] == node_le)
        {
            server.exptable[i] = -1;
        }

        // send WITHDRAW message to all active nodes except the leaving node
        if (server.vz[i].active == 1 && i != x)
        {
            sprintf(buff, "WITHDRAW %s\n", char_node_le);
            if (write(server.vz[i].fd, buff, strlen(buff)) == -1)
            {
                fprintf(stdout, "error writing to socket\n");
                return;
            }
        }
    }
}

/*
 * This function is used when someone disconnects from you. It has four diferent cases, when the node that is leaving is the is an internal node,
 * it simply closes the connection and sets the node as inactive. When the node that is leaving is the root node, is not an anchor and has an internal node,
 * it sets the extern node as the backup node, connects to the new extern node and sends a message to the internal nodes to change the root node.
 * When the node that is leaving is the root node, and does not have an internal node, sets an internal node has an anchor node and sends a message to the
 * internal nodes to change the extern node.
 *
 * Parameters:
 *      x: the index of the node that is leaving the network
 */
void leave(int x)
{
    int intr = 0, i;
    char message[MAX_BUFFER] = "";

    // Find the first active node in the network, starting from index 1.
    // This is used later to determine a new anchor node, if necessary.
    for (intr = 1; intr < MAX_NODES; intr++)
    {
        if (server.vz[intr].active == 1)
            break;
    }

    // Print a message to indicate that the node has left the network,
    // and withdraw its ID and index from the routing table.
    fprintf(stdout, "%s has left network %s\n", server.vz[x].id, server.net);
    withdraw(atoi(server.vz[x].id), x);

    // Close the socket associated with the leaving node's file descriptor.
    close(server.vz[x].fd);

    // If the leaving node is not the first node (index 0), mark it as inactive.
    if (x > 0)
        server.vz[x].active = 0;
    // If the leaving node is not an anchor node,
    // replace the extern node with the backup node (vb), connect to it, and inform
    // the other nodes in the network about the change.
    else if (strcmp(server.my_node.id, server.vb.id) != 0)
    {
        // Replace the leaving node with the backup node.
        server.vz[x] = server.vb;
        server.vz[x].active = 1;

        // Connect to the new node.
        server.vz[x].fd = tcp_client(server.vb.ip, atoi(server.vb.port));

        // Print a message to indicate that the connection was successful.
        fprintf(stdout, "node %s is now connected to node %s\n", server.my_node.id, server.vz[x].id);

        // Inform the node about the new connection.
        sprintf(message, "NEW %s %s %s\n", server.my_node.id, server.my_node.ip, server.my_node.port);
        if (write(server.vz[x].fd, message, strlen(message)) == -1)
        {
            fprintf(stdout, "error writing to socket\n");
            return;
        }

        // Inform the other nodes in the network about the new connection.
        sprintf(message, "EXTERN %s %s %s\n", server.vz[x].id, server.vz[x].ip, server.vz[x].port);

        for (i = 1; i < MAX_NODES; i++)
        {
            if (server.vz[i].active == 1)
            {
                if (write(server.vz[i].fd, message, strlen(message)) == -1)
                {
                    fprintf(stdout, "error writing to socket\n");
                    return;
                }
            }
        }
    }
    // If there is at least one other active node in the network, and the leaving node
    // is an anchor node, choose a new anchor node from the intern nodes and inform
    // the other nodes in the network about the change.
    else if (intr != MAX_NODES)
    {
        // Replace the leaving node with the first intern node in the network.
        server.vz[x] = server.vz[intr];
        server.vz[x].active = 1;

        // Inform the other nodes in the network about the new connection.
        sprintf(message, "EXTERN %s %s %s\n", server.vz[x].id, server.vz[x].ip, server.vz[x].port);

        // Loop through all nodes in the network
        for (i = 1; i < MAX_NODES; i++)
        {
            // Check if the node is active
            if (server.vz[i].active == 1)
            {
                // Send a message to the node indicating that the anchor node has changed
                if (write(server.vz[i].fd, message, strlen(message)) == -1)
                {
                    fprintf(stdout, "error writing to socket\n");
                    return;
                }
            }
        }

        // Print a message indicating that a new anchor node has been chosen
        fprintf(stdout, "choosing node %s as new anchor\n", server.vz[x].id);

        // Set the previous anchor node to inactive
        server.vz[intr].active = 0;
    }
    // If the node is alone in the network
    else
    {
        // Print a message indicating that the node is alone in the network
        fprintf(stdout, "node %s is alone in network %s\n", server.my_node.id, server.net);

        // Set the node as the only active node in the network
        server.vz[x] = server.my_node;
        server.vz[x].active = 0;
    }
}