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
    char connect_ip[16], connect_port[6];
    srand(time(NULL));
    int keyfd = STDIN_FILENO, client_socket, i;
    fd_set rfds_list, rfds;

    for (i = 0; i < MAX_NODES; i++)
    {
        server.vz[i].active = 0;
        server.exptable[i] = 0;
    }

    struct sockaddr_in client_addr;
    socklen_t cli_addr_size = sizeof(client_addr);

    memset(server.names, 0, sizeof(server.names));
    strcpy(server.my_node.ip, argv[1]);
    strcpy(server.my_node.port, argv[2]);

    server.my_node.fd = create_server(server.my_node.ip, atoi(server.my_node.port));
    server.my_node.active = 1;

    if (argc == 5)
    {
        strcpy(connect_ip, argv[3]);
        strcpy(connect_port, argv[4]);
    }
    else if (argc == 3)
    {
        strcpy(connect_ip, SERVER_IP);
        strcpy(connect_port, SERVER_PORT);
    }

    while (1)
    {
        FD_ZERO(&rfds_list);                   // poem todos a 0
        FD_SET(keyfd, &rfds_list);             // adiciona o keyboard
        FD_SET(server.my_node.fd, &rfds_list); // adiciona o server
        for (i = 0; i < MAX_NODES; i++)
        {
            if (server.vz[i].active == 1)
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
                rfds = handle_menu(rfds, argv[1], argv[2], connect_ip, connect_port);
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
                    if (server.vz[i].active == 0)
                    {
                        printf("New connection, socket fd is %d in %d\n", client_socket, i);
                        server.vz[i].fd = client_socket;
                        server.vz[i].active = 1;
                        break;
                    }
                }
            }
            for (int x = 0; x < MAX_NODES; x++)
            {
                if (server.vz[x].active == 1)
                {
                    printf("activated %d in %d", server.vz[x].fd, x);
                    if (FD_ISSET(server.vz[x].fd, &rfds))
                        rfds = client_fd_set(rfds, x);
                }
            }
        }
    }
    return 0;
}