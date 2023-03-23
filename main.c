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
#define MAX_NODES 99
#define SERVER_IP "193.136.138.142" // Change to the IP address of your server
#define SERVER_PORT 59000           // Change to the port number of your server

server_node server;

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int keyfd = STDIN_FILENO, client_socket, i;
    fd_set rfds_list, rfds;

    for (i = 0; i < MAX_NODES; i++)
    {
        server.vz[i].fd = -2;
        server.exptable[i] = 0;
    }

    struct sockaddr_in client_addr;
    socklen_t cli_addr_size = sizeof(client_addr);

    memset(server.names, 0, sizeof(server.names));
    strcpy(server.my_node.ip, argv[1]);
    strcpy(server.my_node.port, argv[2]);

    server.my_node.fd = create_server(server.my_node.ip, atoi(server.my_node.port));

    while (1)
    {
        FD_ZERO(&rfds_list);                   // poem todos a 0
        FD_SET(keyfd, &rfds_list);             // adiciona o keyboard
        FD_SET(server.my_node.fd, &rfds_list); // adiciona o server
        for (i = 0; i < MAX_NODES; i++)
        {
            if (server.vz[i].fd != -2)
                FD_SET(server.vz[i].fd, &rfds_list);
        }
        rfds = rfds_list;
        int ready = select(MAX_NODES + 1, &rfds, NULL, NULL, NULL); // meter o maximo de fd ativos e nao MAX_NODES
        if (ready < 0)
        { /*error*/
            fprintf(stdout, "\n error in select \n");
            exit(1);
        }
        for (int i = 0; i < ready; i++)
        {
            if (FD_ISSET(keyfd, &rfds))
            {
                rfds = handle_menu(rfds, argv[1], argv[2]);
            }
            if (FD_ISSET(server.my_node.fd, &rfds))
            {
                if ((client_socket = accept(server.my_node.fd, (struct sockaddr *)&client_addr, &cli_addr_size)) < 0)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                for (i = 0; i < MAX_NODES; i++)
                {
                    if (server.vz[i].fd == -2)
                    {
                        server.vz[i].fd = client_socket;
                        break;
                    }
                }
            }
            for (int x = 0; x < MAX_NODES; x++)
            {
                if (FD_ISSET(server.vz[x].fd, &rfds))
                    rfds = client_fd_set(rfds, x);
            }
        }
    }
    return 0;
}