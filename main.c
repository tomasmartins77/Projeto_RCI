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

#define max(A, B) ((A) >= (B) ? (A) : (B))
#define MAX_NODES 99
#define SERVER_IP "193.136.138.142" // Change to the IP address of your server
#define SERVER_PORT 59000           // Change to the port number of your server

#include "handle.h"
#include "utility.h"

char buffer[1024];
node_t nodes[MAX_NODES];
server_node server;

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int keyfd = 0, intr = 0, sd, max_sd, client_socket, i;
    fd_set rfds_list, rfds;

    int server_fd, client_fds[MAX_NODES];
    for (i = 0; i < MAX_NODES; i++)
        client_fds[i] = -1;
    struct sockaddr_in client_addr;
    socklen_t cli_addr_size = sizeof(client_addr);

    memset(server.names, 0, sizeof(server.names));
    strcpy(server.my_node.ip, argv[1]);
    strcpy(server.my_node.port, argv[2]);

    server_fd = create_server(server.my_node.ip, atoi(server.my_node.port));

    while (1)
    {
        FD_ZERO(&rfds_list);           // poem todos a 0
        FD_SET(keyfd, &rfds_list);     // adiciona o keyboard
        FD_SET(server_fd, &rfds_list); // adiciona o server
        for (i = 0; i < MAX_NODES; i++)
        {
            sd = client_fds[i];
            if (sd > 0)
            {
                FD_SET(sd, &rfds_list);
            }
        }
        int ready = select(MAX_NODES + 1, &rfds_list, NULL, NULL, NULL); // ve se o keyboard foi set ou n
        if (ready < 0)
        { /*error*/
            fprintf(stdout, "\n error in select \n");
            exit(1);
        }
        for (int i = 0; i < ready; i++)
        {
            if (FD_ISSET(keyfd, &rfds_list) == 1)
            {
                rfds_list = handle_menu(rfds_list, client_fds, server_fd, argv[1], argv[2], intr);
            }
            if (FD_ISSET(server_fd, &rfds_list) == 1)
            {
                if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &cli_addr_size)) < 0)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                FD_SET(client_socket, &rfds_list);
                for (i = 0; i < MAX_NODES; i++)
                {
                    if (client_fds[i] == -1)
                    {
                        client_fds[i] = client_socket;
                        break;
                    }
                }
            }
            for (int x = 0; x < MAX_NODES; x++)
            {
                if (FD_ISSET(client_fds[x], &rfds_list) == 1)
                {
                    rfds_list = client_fd_set(rfds_list, client_fds, x, &intr);
                }
            }
        }
    }
    return 0;
}