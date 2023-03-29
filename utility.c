#include "utility.h"

extern server_node server;
/* This function is designed to unregister all nodes from a  given network using UDP communication */
void clear(char *net, char *connect_ip, char *connect_port)
{
    char message[13] = "", buff[8] = "";
    /* Loop through a maximum number of nodes and unregister from the network */
    for (int i = 0; i < MAX_NODES; i++)
    {
        /* Construct the message to unregister from the network */
        if (i < 10)
            sprintf(message, "UNREG %s 0%d", net, i);
        else
            sprintf(message, "UNREG %s %d", net, i);
        /* Send the unregister message to the specified IP address and port using UDP */
        UDP_connection(message, buff, sizeof(buff), connect_ip, atoi(connect_port));
        /* If the response is not "OKUNREG", exit the program with an error code of 1 */
        if (strcmp(buff, "OKUNREG") != 0)
            exit(1);
    }
    printf("acabei\n");
}

/* This function is designed to request and display a list of nodes on a given network using UDP communication.*/
void show(char *net, char *connect_ip, char *connect_port)
{
    char message[13] = "", buff[2600] = "";

    /* Construct the message to request a list of nodes on the network */
    sprintf(message, "NODES %s", net);
    /* Send the message to the specified IP address and port using UDP */
    UDP_connection(message, buff, sizeof(buff), connect_ip, atoi(connect_port));
    /* Print the response, which should contain a list of nodes on the network */
    printf("%s", buff);
}

/*
 * node_list - Requests and parses a list of nodes on a network using UDP communication
 *
 * This function sends a message to a specified IP address and port using UDP to request a list of nodes
 * on a network identified by a given network name. The response is then parsed and stored in an array
 * of node_t structs passed as a parameter.
 *
 * Parameters:
 *     net: A pointer to a character array containing the name of the network
 *     nodes: An array of node_t structs to store the parsed nodes in
 *     connect_ip: A pointer to a character array containing the IP address to connect to
 *     connect_port: A pointer to a character array containing the port number to connect to
 *
 * Returns:
 *     An integer value representing the result of calling parse_nodes with the contents of buff and the nodes array
 */
int node_list(char *net, node_t *nodes, char *connect_ip, char *connect_port)
{
    char buff[2600] = "";
    char node_msg[10] = "";

    /* Construct the message to request a list of nodes on the network */
    sprintf(node_msg, "NODES %s", net);
    /* Send the message to the specified IP address and port using UDP */
    UDP_connection(node_msg, buff, sizeof(buff), connect_ip, atoi(connect_port));

    /* Parse the list of nodes from the response and store them in the `nodes` array */
    return parse_nodes(buff, nodes);
}

/*
 * parse_nodes - Parses a string containing a list of nodes and stores the results in an array
 *
 * This function takes a string containing a list of nodes separated by newline characters and parses
 * each line into a node_t struct. The resulting nodes are stored in an array passed as a parameter.
 * The function returns the number of nodes parsed and stored in the array.
 *
 * Parameters:
 *     nodes_str: A pointer to a character array containing the list of nodes to parse
 *     nodes: An array of node_t structs to store the parsed nodes in
 *
 * Returns:
 *     An integer value representing the number of nodes parsed and stored in the `nodes` array
 */
int parse_nodes(char *nodes_str, node_t *nodes)
{
    char *line;
    char *token;
    int node_count = 0;
    // make a copy of the input string

    char *nodes_copy = strdup(nodes_str);
    if (nodes_copy == NULL)
        return -1; // error: memory allocation failed

    char *delete = nodes_copy;
    // split string into lines
    line = strtok_r(nodes_copy, "\n", &nodes_copy);
    line = strtok_r(NULL, "\n", &nodes_copy);

    while (line != NULL && node_count < MAX_NODES)
    {
        node_t node;
        // split line into tokens
        token = strtok(line, " ");
        if (token != NULL)
        {
            strncpy(node.id, token, sizeof(node.id));
            node.id[sizeof(node.id) - 1] = '\0';
        }

        token = strtok(NULL, " ");
        if (token != NULL)
        {
            strncpy(node.ip, token, sizeof(node.ip));
            node.ip[sizeof(node.ip) - 1] = '\0';
        }

        token = strtok(NULL, " ");
        if (token != NULL)
        {
            strncpy(node.port, token, sizeof(node.port));
            node.port[sizeof(node.port) - 1] = '\0';
        }

        nodes[node_count++] = node;
        // move to next line
        line = strtok_r(NULL, "\n", &nodes_copy);
    }
    free(delete);
    return node_count;
}
/*
 * This function takes a node ID, a count of nodes, and a list of nodes as input parameters.
 * It iterates through the list of nodes and checks if any node has a matching ID that is not associated with
 * the IP address "0". If a match is found, the function returns 0, indicating that the node is verified.
 * If no match is found, the function returns 1, indicating that the node is not verified.
 *
 * param id The node ID to verify
 * param count The total number of nodes in the list
 * param nodes A pointer to the first element of the list of nodes
 * return 0 if the node is verified, 1 otherwise
 */
int verify_node(char *id, int count, node_t *nodes)
{
    for (int i = 0; i < count; i++)
    {
        if (strcmp(nodes[i].id, id) == 0)
            return 0;
    }
    return 1;
}
/*

*Generates a random integer between 0 and 99 and returns it as a formatted string
*This function generates a random integer between 0 and 99 using the rand() function from the standard
*C library. The resulting integer is then formatted as a string with two digits, using the sprintf()
*function, and stored in a character array passed as a parameter. The function returns the character
*array containing the formatted random number.
*Parameters:
    new_str  A pointer to a character array to store the formatted random number in
*Returns:
    A pointer to the character array containing the formatted random number
*/
char *random_number(char *new_str)
{
    int number = rand() % 100;
    sprintf(new_str, "%02d", number);
    return new_str;
}
/*
 * timeout - Waits for a socket to become ready for reading for a specified amount of time
 *
 * This function sets a timeout value and waits for the specified socket to become ready for reading,
 * using the select() function from the standard C library. If the socket becomes ready before the
 * timeout expires, the function returns. Otherwise, it does not return and the program can continue
 * executing.
 *
 * Parameters:
 *     time: The number of seconds to wait for the socket to become ready
 *     socket: The file descriptor of the socket to wait for
 *
 * Returns:
 *     None
 */

void timeout(int time, int socket)
{
    // Set timeout value
    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;

    // Initialize file descriptor set and add socket to it
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(socket, &rfds);

    // Wait for socket to become ready or timeout to expire
    select(socket + 1, &rfds, NULL, NULL, &timeout);

    // Check if socket is ready for reading
    if (FD_ISSET(socket, &rfds))
        return;
}
/*
 * initialize_nodes - Initializes an array of node_t structs with empty values
 *
 * This function takes an array of node_t structs and initializes each element with empty values for the id,
 * ip, and port fields, using the strcpy() function from the standard C library. The array should be passed
 * as a parameter.
 *
 * Parameters:
 *     nodes: An array of node_t structs to initialize
 *
 * Returns:
 *     None
 */
void inicialize_nodes(node_t *nodes)
{
    for (int i = 0; i < MAX_NODES; i++)
    {
        strcpy(nodes[i].id, "\0");
        strcpy(nodes[i].ip, "\0");
        strcpy(nodes[i].port, "\0");
    }
}
/**
 * Checks if the input string is in the correct format for the specified message
 *
 * Parameters:
 *  input: the input string to be checked
 *  message: the message type to be checked for ("join", "djoin", "leave", "get")
 * Return:
 *  int: 1 if the input is in the correct format, 0 otherwise
 */

int check_input_format(char *input, char *message)
{
    char num1[4], num2[3], num3[3], ip[16], port[6];

    if (strcmp(message, "get") != 0)
    {
        // Extract the first number
        if (sscanf(input + strlen(message) + 1, "%s", num1) != 1)
            return 0;
        // Check if the first number has exactly 3 digits
        if (strlen(num1) != 3)
            return 0;
        // Check if the first number is all digits
        for (int i = 0; i < strlen(num1); i++)
        {
            if (!isdigit(num1[i]))
                return 0;
        }
        // Extract the second number
        if (sscanf(input + strlen(message) + 1 + strlen(num1) + 1, "%s", num2) != 1)
            return 0;
        // Check if the second number has exactly 2 digits
        if (strlen(num2) != 2)
            return 0;
        // Check if the second number is all digits
        for (int i = 0; i < strlen(num2); i++)
        {
            if (!isdigit(num2[i]))
                return 0;
        }
        if (strcmp(message, "djoin") == 0)
        {
            // Extract the third number
            if (sscanf(input + strlen(message) + 1 + strlen(num1) + 1 + strlen(num2) + 1, "%s", num3) != 1)
                return 0;
            // Check if the third number has exactly 2 digits
            if (strlen(num3) != 2)
                return 0;
            // Check if the third number is all digits
            for (int i = 0; i < strlen(num3); i++)
            {
                if (!isdigit(num3[i]))
                    return 0;
            }
            // Extract the IP address
            if (sscanf(input + strlen(message) + 1 + strlen(num1) + 1 + strlen(num2) + 1 + strlen(num3) + 1, "%15s", ip) != 1)
                return 0;
            // Check if the IP address contains only digits, dots, and has at most 3 digits between each dot
            if (!isValidIP(ip))
                return 0;
            // Extract the PORT number
            if (sscanf(input + strlen(message) + 1 + strlen(num1) + 1 + strlen(num2) + 1 + strlen(num3) + 1 + strlen(ip) + 1, "%6s", port) != 1)
                return 0;
            // Check if the PORT number has at most 5 digits
            if (!isValidPort(port))
                return 0;
        }
    }
    else
    {
        // Extract the number
        if (sscanf(input + 4, "%s", num2) != 1)
            return 0;
        // Check if the number has exactly 2 digits
        if (strlen(num2) != 2)
            return 0;
        // Check if the number is all digits
        for (int i = 0; i < strlen(num2); i++)
        {
            if (!isdigit(num2[i]))
                return 0;
        }
    }

    return 1;
}

int check_arguments(int argc, char **argv, char *connect_ip, char *connect_port)
{
    char *ip1 = argv[1];
    char *port1 = argv[2];

    if (argc == 3)
    {
        if (!isValidIP(ip1))
        {
            fprintf(stdout, "Invalid IP address.\n");
            return 1;
        }
        if (!isValidPort(port1))
        {
            fprintf(stdout, "Invalid port number.\n");
            return 1;
        }
        strcpy(server.my_node.ip, ip1);
        strcpy(server.my_node.port, port1);
        strcpy(connect_ip, SERVER_IP);
        strcpy(connect_port, SERVER_PORT);
    }
    else if (argc == 5)
    {
        char *ip2 = argv[3];
        char *port2 = argv[4];
        // validate inputs
        if (!isValidIP(ip1))
        {
            fprintf(stdout, "Invalid IP address.\n");
            return 1;
        }
        if (!isValidPort(port1))
        {
            fprintf(stdout, "Invalid port number.\n");
            return 1;
        }
        if (!isValidIP(ip2))
        {
            fprintf(stdout, "Invalid IP address.\n");
            return 1;
        }
        if (!isValidPort(port2))
        {
            fprintf(stdout, "Invalid port number.\n");
            return 1;
        }
        strcpy(connect_ip, argv[3]);
        strcpy(connect_port, argv[4]);
    }
    else
        return 1;

    return 0;
}

int isValidIP(char *ip)
{
    char *tok;
    char *ip_aux = strdup(ip);
    if (ip_aux == NULL)
        exit(1);

    char *delete = ip_aux;

    tok = strtok(ip_aux, ".");
    int num_dots = 0;

    while (tok != NULL)
    {
        num_dots++;
        // Check if the token is all digits
        for (int i = 0; i < strlen(tok); i++)
        {
            if (!isdigit(tok[i]))
            {
                return 0;
            }
        }
        // Check if the token is between 0 and 255
        int num = atoi(tok);
        if (num < 0 || num > 255)
            return 0;
        tok = strtok(NULL, ".");
    }
    num_dots--;
    if (num_dots != 3)
        return 0;
    free(delete);
    return 1;
}

int isValidPort(char *port)
{
    int len = strlen(port);

    // check length
    if (len > 6)
        return 0;

    // check for non-numeric characters
    for (int i = 0; i < len; i++)
    {
        if (!isdigit(port[i]))
            return 0;
    }

    // convert port string to integer
    int port_int = atoi(port);

    // check if port is in valid range
    if (port_int < 0 || port_int > 65535)
        return 0;

    return 1;
}