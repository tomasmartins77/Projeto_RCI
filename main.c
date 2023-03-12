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
server_node server;

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int position = 0, intr = 0;
    fd_set rfds_list;
    int keyfd = 0, count = 0, flag_join = 0, flag_delete = 0, flag_create = 0;
    char buff[1024], str_temp[10], id_temp[3], ip_temp[16], port_temp[6];
    char message[10], arg1[9], arg2[5], bootid[7], bootIP[7], bootTCP[8], net[4];
    node_t temp;
    int server_fd, client_fds[MAX_NODES] = {-1};
    struct sockaddr_in client_addr;
    socklen_t cli_addr_size = sizeof(client_addr);

    memset(server.names, 0, sizeof(server.names));
    server_fd = create_server(argv[1], atoi(argv[2]));

    while (1)
    {
        FD_ZERO(&rfds_list);           // poem todos a 0
        FD_SET(keyfd, &rfds_list);     // adiciona o keyboard
        FD_SET(server_fd, &rfds_list); // adiciona o server

        int ready = select(MAX_NODES + 1, &rfds_list, NULL, NULL, NULL); // ve se o keyboard foi set ou n
        if (ready < 0)
        { /*error*/
            fprintf(stdout, "\n error in select \n");
            exit(1);
        }

        if (FD_ISSET(keyfd, &rfds_list) == 1)
        {
            fgets(buff, 255, stdin); // LE o que ta escrito
            sscanf(buff, "%s %s %s %s %s %s", message, arg1, arg2, bootid, bootIP, bootTCP);
            if (strcmp(message, "join") == 0 && flag_join == 0)
            {
                count = handle_join(arg1, arg2, argv[1], argv[2], position, client_fds);
                strcpy(net, arg1);
                if (count > 0)
                    FD_SET(client_fds[position++], &rfds_list);
                flag_join = 1;
            }
            else if (strcmp(message, "join") == 0 && flag_join == 1)
                fprintf(stdout, "node already created\n");
            if (strcmp(message, "leave") == 0 && flag_join == 1)
            {
                flag_join = 0;
                handle_leave(net, server.my_node.id, position, client_fds);
            }
            else if (strcmp(message, "leave") == 0 && flag_join == 0)
                fprintf(stdout, "no node created\n");
            if (strcmp(message, "djoin") == 0)
                handle_djoin(arg1, arg2, bootid, bootIP, bootTCP);
            if (strcmp(message, "create") == 0)
                flag_create = handle_create(arg1);
            if (strcmp(message, "delete") == 0 && flag_create > 0)
                handle_delete(arg1);
            if (strcmp(message, "get") == 0)
                handle_get(arg1, arg2);
            if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag_join == 1)
                handle_st(intr);
            else if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag_join == 0)
                fprintf(stdout, "no node created\n");
            if (strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0)
                handle_sn();
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
                int bytes_received = read(client_fds[x], buff, 1024);
                if (bytes_received == 0)
                {
                    printf("asdasdasd\n");
                    // else if x pertence a intr -> intr = intr - x
                    if (strcmp(server.my_node.id, server.VB.id) != 0)
                    {
                        server.VE = server.VB;
                        sprintf(buffer, "NEW %s %s %s\n", server.my_node.id, server.my_node.ip, server.my_node.port);
                        write(client_fds[x], buff, 1024);
                        sprintf(buff, "EXTERN %s %s %s\n", server.VE.id, server.VE.ip, server.VE.port);
                        // for loop a enviar EXTERN aos intr
                        for (int i = 0; i < intr; i++)
                        {
                        }
                    }
                    // else if intr != NULL -> escolhe intr random e EX = intr random
                    //  envia EXTERN para intr     intr = intr - intr random
                    else
                    {
                        server.VE = server.my_node;
                    }
                    FD_CLR(client_fds[x], &rfds_list);
                }
                fprintf(stdout, "%s", buff);
                sscanf(buff, "%s %s %s %s", str_temp, temp.id, temp.ip, temp.port);

                if (strcmp(str_temp, "NEW") == 0)
                {
                    if (strcmp(server.my_node.id, server.VE.id) == 0) // ancora
                    {
                        server.VE = temp;
                    }
                    else
                    {
                        server.VI[intr++] = temp;
                    }
                    sprintf(buff, "EXTERN %s %s %s\n", server.VE.id, server.VE.ip, server.VE.port);
                    write(client_fds[x], buff, 1024);
                }
                if (strcmp(str_temp, "EXTERN") == 0)
                {
                    server.VB = temp;
                }
            }
        }
    }
    return 0;
}