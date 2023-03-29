#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <time.h>

#include "handle.h"
#include "utility.h"
#include "connections.h"
#include "fd_functions.h"

#define max(A, B) ((A) >= (B) ? (A) : (B))

server_node server;

int main(int argc, char *argv[])
{
    // Declare and initialize some variables
    char connect_ip[16] = "", connect_port[6] = "";
    srand(time(NULL));
    int keyfd = STDIN_FILENO, client_socket, i, max_fd;
    fd_set rfds_list, rfds;
    for (i = 0; i < MAX_NODES; i++)
    {
        server.vz[i].active = 0;
        server.vz[i].bytes_received = 0;
        server.exptable[i] = -1;
    }

    struct sockaddr_in client_addr;
    socklen_t cli_addr_size = sizeof(client_addr);

    // Check command line arguments and initialize the server
    if (check_arguments(argc, argv, connect_ip, connect_port) == 1)
    {
        fprintf(stdout, "Exiting...\n");
        exit(1);
    }
    server.my_node.fd = create_server(server.my_node.ip, atoi(server.my_node.port));
    server.my_node.active = 1;
    memset(server.names, 0, sizeof(server.names));

    // Start an infinite loop to handle input/output events
    while (1)
    {
        // Initialize a set of file descriptors to wait for input
        FD_ZERO(&rfds_list);
        FD_SET(keyfd, &rfds_list);
        FD_SET(server.my_node.fd, &rfds_list);
        max_fd = max(keyfd, server.my_node.fd);
        for (i = 0; i < MAX_NODES; i++)
        {
            if (server.vz[i].active == 1)
            {
                max_fd = max(max_fd, server.vz[i].fd);
                FD_SET(server.vz[i].fd, &rfds_list);
            }
        }

        // Wait for input on one or more file descriptors
        rfds = rfds_list;
        int ready = select(max_fd + 1, &rfds, NULL, NULL, NULL);
        if (ready < 0)
        {
            // Handle errors
            fprintf(stdout, "\n error in select, something very bad happened\n");
            exit(1);
        }

        // Process input events
        for (int i = 0; i < ready; i++)
        {
            // Handle input from the keyboard
            if (FD_ISSET(keyfd, &rfds))
                rfds = handle_menu(rfds, argv[1], argv[2], connect_ip, connect_port);

            // Handle incoming connections to the server
            if (FD_ISSET(server.my_node.fd, &rfds))
            {
                if ((client_socket = accept(server.my_node.fd, (struct sockaddr *)&client_addr, &cli_addr_size)) < 0)
                {
                    fprintf(stdout, "Error accepting connection, trying again\n");
                    continue;
                }
                for (i = 0; i < MAX_NODES; i++)
                {
                    //  accept the connection and adds it to the first position of the list
                    if (server.vz[i].active == 0)
                    {
                        server.vz[i].fd = client_socket;
                        server.vz[i].active = 1;
                        break;
                    }
                }
            }

            // Handle input from clients
            for (int x = 0; x < max_fd + 1; x++)
            {
                if (server.vz[x].active == 1)
                {
                    if (FD_ISSET(server.vz[x].fd, &rfds))
                        rfds = client_fd_set(rfds, x);
                }
            }
        }
    }
    return 0;
}