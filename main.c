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

#define PORT 8081
#define MAX_NODES 99
#define SERVER_IP "193.136.138.142" // Change to the IP address of your server
#define SERVER_PORT 59000           // Change to the port number of your server

#include "handle.h"
#include "utility.h"

char buffer[1024];
node_t nodes[MAX_NODES];
server_node this_node;
int client_fds[MAX_NODES];
server_node server;

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int position = 0;
    fd_set rfds_list;
    int keyfd = 0, count = 0, flag = 0;
    char buff[1024], str_temp[10], id_temp[3], ip_temp[16], port_temp[6];
    char message[10], arg1[9], arg2[5], bootid[7], bootIP[7], bootTCP[8], net[4];
    node_t temp;
    int server_fd, client_fds[MAX_NODES] = {-1};
    struct sockaddr_in client_addr;
    socklen_t cli_addr_size = sizeof(client_addr);

    inicialize_node();

    server_fd = create_server(argv[1], atoi(argv[2]));

    while (1)
    {
        FD_ZERO(&rfds_list);           // poem todos a 0
        FD_SET(keyfd, &rfds_list);     // adiciona o keyboard
        FD_SET(server_fd, &rfds_list); // adiciona o server

        int ready = select(MAX_NODES + 1, &rfds_list, NULL, NULL, NULL); // ve se o keyboard foi set ou n
        if (ready < 0)
        { /*error*/
            printf("oh no\n");
            exit(1);
        }

        if (FD_ISSET(keyfd, &rfds_list) == 1)
        {
            fgets(buff, 255, stdin); // LE o que ta escrito
            sscanf(buff, "%s %s %s %s %s %s", message, arg1, arg2, bootid, bootIP, bootTCP);
            if (strcmp(message, "join") == 0)
            {
                count = handle_join(arg1, arg2, argv[1], argv[2], position, client_fds);
                strcpy(net, arg1);
                if (count > 0)
                    FD_SET(client_fds[position++], &rfds_list);
                flag = 1;
            }
            if (strcmp(message, "leave") == 0 && flag == 1)
            {
                handle_leave(net, server.my_node.id, position, client_fds, server_fd);
                for (int i = 0; i < position; i++)
                    FD_CLR(client_fds[i], &rfds_list);
            }
            else if (strcmp(message, "leave") == 0 && flag == 0)
                fprintf(stdout, "no node created\n");
            if (strcmp(message, "djoin") == 0)
                handle_djoin(arg1, arg2, bootid, bootIP, bootTCP);
            if (strcmp(message, "create") == 0)
                handle_create(arg1);
            if (strcmp(message, "delete") == 0)
                handle_delete(arg1);
            if (strcmp(message, "get") == 0)
                handle_get(arg1, arg2);
            if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag == 1)
                handle_st();
            else if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag == 0)
                fprintf(stdout, "no node created\n");
            if (strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0)
                handle_sn(arg2);
            if (strcmp(message, "show routing") == 0 || strcmp(message, "sr") == 0)
                handle_sr(arg2);
            if (strcmp(message, "exit") == 0)
            {
                close(server_fd);
                FD_CLR(server_fd, &rfds_list);
                fprintf(stdout, "exiting program\n");
                exit(1);
            }
            if (strcmp(message, "clear") == 0)
            {
                clear(arg1);
                exit(1);
            }
        }
        if (FD_ISSET(server_fd, &rfds_list) == 1)
        {
            if ((client_fds[position] = accept(server_fd, (struct sockaddr *)&client_addr, &cli_addr_size)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            FD_SET(client_fds[position++], &rfds_list);
        }
        for (int x = 0; x < (position - 1); x++)
        {
            if (FD_ISSET(client_fds[x], &rfds_list) == 1)
            {
                memset(buff, 0, 1024);
                read(client_fds[x], buff, 1024);
                fprintf(stdout, "%s", buff);
                sscanf(buff, "%s %s %s %s", str_temp, id_temp, ip_temp, port_temp);

                if (strcmp(str_temp, "NEW") == 0)
                {
                    if (strcmp(server.my_node.id, server.VE.id) == 0) // ancora
                    {
                        strcpy(server.VE.id, id_temp);
                        strcpy(server.VE.ip, ip_temp);
                        strcpy(server.VE.port, port_temp);
                    }
                    else
                    {
                        printf("nao\n");
                        // colocar como interno
                    }
                    sprintf(buff, "EXTERN %s %s %s\n", server.VE.id, server.VE.ip, server.VE.port);
                    write(client_fds[x], buff, 1024);
                }
                if (strcmp(str_temp, "EXTERN") == 0)
                {
                    strcpy(server.VB.id, id_temp);
                    strcpy(server.VB.ip, ip_temp);
                    strcpy(server.VB.port, port_temp);
                }
            }
        }
    }
    return 0;
}