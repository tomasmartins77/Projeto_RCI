#include "connections.h"

extern server_node server;
/*
*Function: UDP_server_message
*Brief:
*   UDP server message function sends a message to a server and waits for a response.
*Parameters:
*   message: pointer to the message to be sent to the server
*   response: pointer to the buffer to store the response from the server
*   len: length of the response buffer
*   connect_ip: IP address of the server
*   connect_port: port number of the server
*Return Value:
*   0 on success
*   -1 on error
*/
int UDP_server_message(char *message, char *response, int len, char *connect_ip, int connect_port)
{
    int sockfd;
    struct sockaddr_in server_addr;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        fprintf(stdout, "socket error\n");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(connect_ip);
    server_addr.sin_port = htons(connect_port);

    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        fprintf(stdout, "sendto error, couldn't connect to node server\n");
        return -1;
    }
    timeout(2, sockfd);
    int n = recvfrom(sockfd, response, len, 0, NULL, NULL);
    if (n < 0)
    {
        fprintf(stdout, "recvfrom error, couldn't connect to node server\n");
        return -1;
    }

    response[n] = '\0';
    close(sockfd);

    return 0;
}
/*
*Function: UDP_connection
*Brief:
*   UDP connection function sends a message to a server and waits for a response.
*   If the first attempt fails, the function tries to connect to a default server.
*Parameters:
*   message: pointer to the message to be sent to the server
*   response: pointer to the buffer to store the response from the server
*   len: length of the response buffer
*   connect_ip: IP address of the server
*   connect_port: port number of the server
*  Return Value:
*   none
*/
void UDP_connection(char *message, char *response, int len, char *connect_ip, int connect_port)
{
    if (UDP_server_message(message, response, len, connect_ip, connect_port) == -1)
    {
        printf("connecting to ip %s and port %s\n", SERVER_IP, SERVER_PORT);
        if (UDP_server_message(message, response, len, SERVER_IP, atoi(SERVER_PORT)) == -1)
            printf("Error: could not connect to server, server is probably down\n");
    }

    return;
}
/*
*Function: tcp_client
*Brief:
*    TCP client function creates a socket and connects to a server at the specified IP address and port number.
*Parameters:
*   ip_address: IP address of the server
*   portno: port number of the server
*Return Value:
*   socket file descriptor on success
*   -1 on error
*/
int tcp_client(char *ip_address, int portno)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stdout, "\n Socket creation error\n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0)
    {
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        close(sockfd);
        return -1;
    }

    return sockfd;
}
/*
*Function: create_server
*Brief:
*The create_server function creates a socket, binds it to the specified IP address and port number,
*and listens for incoming connections from clients.
*Parameters:
*   ip_address: IP address of the server
*   port: port number of the server
*Return Value:
*server file descriptor on success
*-1 on error
*/
int create_server(char *ip_address, int port)
{
    int server_fd;
    struct sockaddr_in server_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stdout, "socket error, pls choose a new parameters\n");
        return -1;
    }
    int opt = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        fprintf(stdout, "setsockopt error, pls choose a new parameters\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        fprintf(stdout, "bind error, pls choose a new parameters\n");
        return -1;
    }

    if (listen(server_fd, MAX_NODES) == -1)
    {
        fprintf(stdout, "listen error, pls choose a new parameters\n");
        return -1;
    }
    return server_fd;
}