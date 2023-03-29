#include "handle.h"

extern server_node server;

/*
 *
 *  This function handles the joining of a node to a network, sets the node's status to active, and connects to the boot node.
 *  If the joining node is the first node in the network, it will connect to itself.
 *  If the joining node is not the first node in the network, it will connect to a random node in the network.
 *  If all nodes in the network are unavailable, it will connect to the local node.
 *
 *  Parameters:
 *      net - the network name
 *      id - the node ID
 *      connect_ip - the IP address of the boot node
 *      connect_port - the port of the boot node
 *
 *  Returns:
 *     count - the number of nodes in the network
 */
int handle_join(char *net, char *id, char *connect_ip, char *connect_port)
{
    // Declare an array of node_t structures with size MAX_NODES and initialize it
    node_t nodes[MAX_NODES];
    inicialize_nodes(nodes);

    // See how many nodes are in the network
    int count = node_list(net, nodes, connect_ip, connect_port);

    // Declare local variables
    int int_connect = 0, i = 0;
    char id_temp[3] = "", message[50] = "", response[6] = "";

    // If there are nodes in the network
    if (count > 0)
    {
        // Loop until a connection is made to a random node in the network
        while (i != count)
        {
            // Generate a random index for the node array
            int_connect = rand() % count;

            // Node is unavailable, look for another available node
            if (strcmp(nodes[int_connect].ip, "\0") == 0)
                continue;

            // Loop until a unique node ID is generated
            while (verify_node(id, count, nodes) == 0)
            {
                strcpy(id_temp, random_number(id));
                strcpy(id, id_temp);
            }

            // Try to connect to the selected node using the handle_djoin function
            if (handle_djoin(net, id, nodes[int_connect].id, nodes[int_connect].ip, nodes[int_connect].port, connect_ip, connect_port) == 0)
            {
                // If the connection is successful, break out of the loop
                fprintf(stdout, "node %s is connected to node %s\n", server.my_node.id, server.vz[0].id);
                break;
            }

            // Mark the selected node as unavailable and look for another available node
            strcpy(nodes[int_connect].ip, "\0");
            for (i = 0; i < count; i++)
            {
                if (strcmp(nodes[i].ip, "\0") != 0)
                    break;
            }
        }
    }

    // If there are no nodes in the network or all nodes are unavailable, connect to the local node
    if (count == 0 || i == count)
    {
        handle_djoin(net, id, id, server.my_node.ip, server.my_node.port, connect_ip, connect_port);
    }

    // Send a message to the selected node to register the new node in the network
    sprintf(message, "REG %s %s %s %s", net, id, server.my_node.ip, server.my_node.port);
    UDP_connection(message, response, sizeof(response), connect_ip, atoi(connect_port));

    // If the registration is successful, print a message
    if (strcmp(response, "OKREG") == 0)
        fprintf(stdout, "Node %s is correctly registered in network %s\n", server.my_node.id, server.net);

    // Return the number of nodes in the network
    return count;
}

/*
 *
 *  This function handles the joining of a node to a network, sets the node's status to active, and connects to the boot node.
 *  If the joining node is the first node in the network, puts it as an anchor.
 *  If the joining node is not the first node in the network, connects to the boot node and sends a message with the new node's information.
 *
 *  Parameters:
 *      net: a char pointer representing the network name.
 *      id: a char pointer representing the node ID.
 *      bootid: a char pointer representing the boot node ID.
 *      bootIP: a char pointer representing the boot node IP address.
 *      bootTCP: a char pointer representing the boot node TCP port.
 *      connect_ip: a char pointer representing the IP address of the node that the joining node will connect to.
 *      connect_port: a char pointer representing the TCP port of the node that the joining node will connect to.
 * Returns:
 *     0 if the connection to the boot node is successful.
 *    -1 if the connection to the boot node fails.
 */
int handle_djoin(char *net, char *id, char *bootid, char *bootIP, char *bootTCP, char *connect_ip, char *connect_port)
{
    char message[MAX_BUFFER] = "";
    // Copies the id to the server's node id.
    strcpy(server.my_node.id, id);

    // If the joining node is not the first node in the network.
    if (strcmp(id, bootid) != 0)
    {
        // If the boot node's IP and TCP are the same as the joining node's IP and TCP, return an error.
        if (strcmp(bootIP, server.my_node.ip) == 0 && strcmp(bootTCP, server.my_node.port) == 0)
            return -1;

        // Connects to the boot node.
        server.vz[0].fd = tcp_client(bootIP, atoi(bootTCP));

        // If the connection to the boot node fails, return an error.
        if (server.vz[0].fd < 0)
            return -1;

        // Sets the boot node as active.
        server.vz[0].active = 1;

        // Sends a message to the boot node with the new node's information.
        sprintf(message, "NEW %s %s %s\n", id, server.my_node.ip, server.my_node.port);
        if (write(server.vz[0].fd, message, strlen(message)) == -1)
            return -1;
    }
    // If the joining node is the first node in the network.
    else
        fprintf(stdout, "Node %s is the first node in network %s\n", server.my_node.id, server.net);

    // Sets the boot node's information in the server's node list.
    strcpy(server.vz[0].id, bootid);
    strcpy(server.vz[0].ip, bootIP);
    strcpy(server.vz[0].port, bootTCP);

    // Set backup as my node.
    server.vb = server.my_node;

    // Returns 0 to indicate success.
    return 0;
}

/*
 *
 *  This function handles the leaving of a node from a network, sets the node's status to inactive, and closes the connection.
 *
 *  Parameters:
 *   net: a char pointer representing the network name.
 *   id: a char pointer representing the node ID.
 *   connect_ip: a char pointer representing the IP address the node connected to.
 *   connect_port: a char pointer representing the port the node connected to.
 *   flag: an integer representing whether or not to unregister the node from the network, if joined by command join.
 *
 */
void handle_leave(char *net, char *id, char *connect_ip, char *connect_port, int flag)
{
    char message[13] = "", response[8] = "";

    // Loop through all nodes in the network
    for (int i = 0; i < MAX_NODES; i++)
    {
        // Clear the names created by the node
        strcpy(server.names[i], "\0");

        // If the node is active, close its socket and clear its information
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

    // clear routing table
    handle_cr();

    // If flag is 1, it was created by join command
    if (flag == 1)
    {
        sprintf(message, "UNREG %s %s", net, id);
        UDP_connection(message, response, sizeof(response), connect_ip, atoi(connect_port));

        // If the response is not "OKUNREG", print an error message
        if (strcmp(response, "OKUNREG") != 0)
            fprintf(stdout, "node %s is not correctly unregistered in network %s\n", server.my_node.id, server.net);
    }

    // Print a message indicating that the node has left the network
    fprintf(stdout, "%s left network %s\n", server.my_node.id, server.net);

    // Clear the network and node information
    strcpy(server.net, "\0");
    strcpy(server.my_node.id, "\0");
}

/*
 *
 *  This function handles the creation of a file in the file array.
 *
 *  Parameters:
 *   name: a char pointer representing the name of the file to be created.
 *   flag: an integer representing whether or not the file was created.
 *
 * Returns:
 *   flag: an integer representing whether or not the file was created.
 */
int handle_create(char *name, int flag)
{
    int i;

    // If the name of the file to be created is empty, print an error message
    if (strcmp(name, "\0") == 0)
    {
        fprintf(stdout, "no name to create file\n");
        return flag;
    }

    // Iterate over the list of file names in the server's names array.
    for (i = 0; i < MAX_NODES; i++)
    {
        // If the file already exists, print an error message
        if (strcmp(server.names[i], name) == 0)
        {
            fprintf(stdout, "file already exists: %s\n", name);
            return flag;
        }

        // If an empty slot is found in the file system, add the new file
        if (strcmp(server.names[i], "\0") == 0)
        {
            strcpy(server.names[i], name);
            flag++;
            break;
        }
    }

    // If the loop finishes without finding an empty slot,
    // print an error message indicating that no space is available.
    if (flag == 0)
        fprintf(stdout, "no space to create file: %s\n", name);
    // Otherwise, print a message indicating that the file has been created.
    else
        fprintf(stdout, "created file: %s\n", server.names[i]);

    // Return the updated `flag` value.
    return flag;
}

/*
 *
 *  This function handles the deletion of a file in the file array.
 *
 *  Parameters:
 *   name: a char pointer representing the name of the file to be deleted.
 *
 */
void handle_delete(char *name)
{
    int flag = 0;
    // Loop through all the files in the server and try to find the one to delete
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (strcmp(server.names[i], name) == 0) // If the file is found
        {
            strcpy(server.names[i], "\0"); // Delete the file by setting its name to empty
            flag = 1;
            fprintf(stdout, "deleted file: %s\n", name); // Print a message confirming that the file was deleted
            break;                                       // Stop searching for the file once it is found and deleted
        }
    }
    if (flag == 0)
    {
        // If the file is not found, print an error message indicating that it cannot be deleted
        fprintf(stdout, "no file to be deleted, %s doesnt exist\n", name);
    }
}

/*
 * This function handles the command get, if the destination node is the current node, it checks if the file exists, if it does,
 * it returns 1, if it doesn't, it returns 2.
 * If the destination node is not the current node, it checks if there is an entry in the server's forwarding table for the destination node,
 * if there is, it sends a QUERY message to the next node in the forwarding table.
 * If there is no entry in the forwarding table, it sends a QUERY message to all node's neighbours but not the one it came from.
 *
 *  Parameters:
 *      dest: a char pointer representing the destination node.
 *      name: a char pointer representing the name of the file to be retrieved.
 *      origin: a char pointer representing the node that sent the message.
 *      x: an integer representing the position of the current node.
 *
 * Returns:
 *     flag: an integer representing whether or not the file exists.
 *     0: if everything went well.
 */
int handle_get(char *dest, char *name, char *origin, int x)
{
    int flag = 2;

    // If the current node is the destination node, check if the requested file exists.
    if (strcmp(dest, server.my_node.id) == 0)
    {
        for (int i = 0; i < MAX_NODES; i++)
        {
            if (strcmp(name, server.names[i]) == 0) // file exists
                flag = 1;
        }
        return flag;
    }

    char buff[MAX_BUFFER] = "";
    int path = 0;
    int dest_int = atoi(dest);

    // Check if there is an entry in the server's forwarding table for the destination node.
    if (server.exptable[dest_int] != -1)
    {
        // If there is, send a QUERY message to the next node in the forwarding path.
        sprintf(buff, "QUERY %s %s %s \n", dest, origin, name);
        path = server.exptable[dest_int];
        for (int i = 0; i < MAX_NODES; i++)
        {
            if (atoi(server.vz[i].id) == path)
            {
                // Send the message to the next node.
                if (write(server.vz[i].fd, buff, strlen(buff)) == -1)
                {
                    fprintf(stdout, "error writing to socket\n");
                    return -1;
                }
                break;
            }
        }
    }
    else
    {
        // If there is no entry in the forwarding table, broadcast the QUERY message to all active nodes except the current one.
        sprintf(buff, "QUERY %s %s %s\n", dest, origin, name);
        for (int i = 0; i < MAX_NODES; i++)
        {
            if (server.vz[i].active == 1 && i != x)
            {
                // Add an entry to the forwarding table for each active node that receives the message.
                server.exptable[atoi(server.vz[i].id)] = atoi(server.vz[i].id);
                // Send the message to the active node.
                if (write(server.vz[i].fd, buff, strlen(buff)) == -1)
                {
                    fprintf(stdout, "error writing to socket\n");
                    return -1;
                }
            }
        }
    }

    return 0;
}

/*
 * This function shows the current node's neighbours.
 *
 */
void handle_st()
{
    fprintf(stdout, "My node is %s, here are my neighbours:\n", server.my_node.id);
    fprintf(stdout, "Extern: %s %s %s\n", server.vz[0].id, server.vz[0].ip, server.vz[0].port);
    fprintf(stdout, "Backup: %s %s %s\n", server.vb.id, server.vb.ip, server.vb.port);
    fprintf(stdout, "Interns:\n");
    for (int i = 1; i < MAX_NODES; i++)
    {
        if (server.vz[i].active == 1)
            fprintf(stdout, "-> %s %s %s\n", server.vz[i].id, server.vz[i].ip, server.vz[i].port);
    }
    fprintf(stdout, "\n\n");
}

/*
 * This function shows the current node's files.
 *
 */
void handle_sn()
{
    fprintf(stdout, "Here are the files of node %s:\n", server.my_node.id);
    for (int i = 0; i < 50; i++)
    {
        if (strcmp(server.names[i], "\0") != 0)
        {
            fprintf(stdout, "%s\n", server.names[i]);
        }
    }
    fprintf(stdout, "\n\n");
}

/*
 * This function shows the current node's forwarding table.
 *
 */
void handle_sr()
{
    fprintf(stdout, "Here is the expedition table of node %s:\n", server.my_node.id);
    fprintf(stdout, "destination --> neighbour\n");
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (server.exptable[i] != -1)
        {
            printf("     %02d     -->    %02d  \n", i, server.exptable[i]);
        }
    }
    fprintf(stdout, "\n\n");
}

/*
 * This function clears the current node's forwarding table.
 *
 */
void handle_cr()
{
    for (int i = 0; i < MAX_NODES; i++)
    {
        server.exptable[i] = -1; // if the entry is -1, it means that there is no entry for that node
    }
}