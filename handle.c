#include "handle.h"

extern node_t nodes[MAX_NODES];
extern server_node server;
int exptable[100]={0};

int handle_join(char *net, char *id)
{
    node_t nodes[MAX_NODES];

    int count = node_list(net, 0, nodes);
    int int_connect, flag = 1;

    if (count > 0) // bananninhas das boas
    {
        int_connect = rand() % count;
        while (verify_node(id, count, nodes) == 0)
            strcpy(id, random_number(id));

        handle_djoin(net, id, nodes[int_connect].id, nodes[int_connect].ip, nodes[int_connect].port);
    }
    else
        handle_djoin(net, id, id, server.my_node.ip, server.my_node.port);

    return count;
}

void handle_djoin(char *net, char *id, char *bootid, char *bootIP, char *bootTCP)
{
    char message[50] = "", response[6];
    strcpy(server.my_node.id, id);
    if (strcmp(id, bootid) != 0)
    {
        server.vz[0].fd = tcp_client(bootIP, atoi(bootTCP));

        sprintf(message, "NEW %s %s %s\n", id, server.my_node.ip, server.my_node.port);
        write(server.vz[0].fd, message, strlen(message));
        
    }
    else
        server.vz[0].fd = -1;

    strcpy(server.vz[0].id, bootid);
    strcpy(server.vz[0].ip, bootIP);
    strcpy(server.vz[0].port, bootTCP);

    server.vb = server.my_node;

    sprintf(message, "REG %s %s %s %s", net, id, server.my_node.ip, server.my_node.port);
    UDP_server_message(message, 1, response, sizeof(response));
    if (strcmp(response, "OKREG") != 0)
        exit(1);
}

void handle_leave(char *net, char *id)
{
    char message[13], response[8];
    for (int i = 0; i < MAX_NODES; i++)
    {
        if (server.vz[i].fd != -1)
        {
            close(server.vz[i].fd);
            server.vz[i].fd = -1;
        }
    }

    sprintf(message, "UNREG %s %s", net, id);
    UDP_server_message(message, 1, response, sizeof(response));
    if (strcmp(response, "OKUNREG") != 0)
        exit(1);
}

int handle_create(char *name)
{
    int i, flag = 0;
    for (i = 0; i < 50; i++)
    {
        if (strcmp(server.names[i], "\0") == 0)
        {
            strcpy(server.names[i], name);
            flag++;
            break;
        }
    }
    fprintf(stdout, "created file: %s\n", server.names[i]);
    return flag;
}

void handle_delete(char *name)
{
    int i, flag = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        if (strcmp(server.names[i], name) == 0)
        {
            strcpy(server.names[i], "\0");
            flag = 1;
            fprintf(stdout, "deleted file: %s\n", name);
            break;
        }
    }
    if (flag == 0)
    {
        fprintf(stdout, "no file deleted\n");
    }
}

int handle_get(char *dest, char *name)
{

   int dad=0;
   dad=dad_get(dest,name,server.my_node.id);
   return dad;
}

void handle_st()
{
    fprintf(stdout, "Vizinho externo: %s %s %s\n", server.vz[0].id, server.vz[0].ip, server.vz[0].port);
    fprintf(stdout, "Vizinho Backup: %s %s %s\n", server.vb.id, server.vb.ip, server.vb.port);
    fprintf(stdout, "Vizinhos internos:\n");
    for (int i = 1; i < MAX_NODES; i++)
    {
        if (server.vz[i].fd != -1)
            fprintf(stdout, "%s %s %s\n", server.vz[i].id, server.vz[i].ip, server.vz[i].port);
    }
}

void handle_sn()
{
    int i;
    fprintf(stdout, "files:\n");
    for (i = 0; i < 50; i++)
    {
        if (strcmp(server.names[i], "\0") != 0)
        {
            fprintf(stdout, "%s\n", server.names[i]);
        }
    }
}

void handle_sr(char *net)
{ 
  for (int i = 0; i < 99; i++)
   {
       if (exptable[i]!=0)
       {
           printf("%d-->%d\n",i,exptable[i]);
       }
   }/* function code here */
}

fd_set handle_menu(fd_set rfds_list, char *ip, char *port)
{
    char buff[1024], str_temp[10], id_temp[3], ip_temp[16], port_temp[6];
    char message[10], arg1[9], arg2[5], bootid[7], bootIP[7], bootTCP[8];
    static int flag_join = 0, flag_delete = 0, flag_create = 0;
    int count = 0;

    fgets(buff, 255, stdin); // LE o que ta escrito
    sscanf(buff, "%s %s %s %s %s %s", message, arg1, arg2, bootid, bootIP, bootTCP);
    if (strcmp(message, "join") == 0 && flag_join == 0)
    {
        strcpy(server.net, arg1);
        count = handle_join(arg1, arg2);

        if (count > 0)
            FD_SET(server.vz[0].fd, &rfds_list);
        flag_join = 1;
    }
    else if (strcmp(message, "join") == 0 && flag_join == 1)
        fprintf(stdout, "node already created\n");
    if (strcmp(message, "djoin") == 0 && flag_join == 0)
    {
        strcpy(server.net, arg1);
        handle_djoin(arg1, arg2, bootid, bootIP, bootTCP);

        if (count > 0)
            FD_SET(server.vz[0].fd, &rfds_list);
        flag_join = 1;
    }
    else if (strcmp(message, "djoin") == 0 && flag_join == 1)
        fprintf(stdout, "node already created\n");
    if (strcmp(message, "leave") == 0 && flag_join == 1)
    {
        flag_join = 0;
        handle_leave(server.net, server.my_node.id);
    }
    else if (strcmp(message, "leave") == 0 && flag_join == 0)
        fprintf(stdout, "no node created\n");

    if (strcmp(message, "create") == 0 && flag_join == 1)
        flag_create = handle_create(arg1);
    else if (strcmp(message, "create") == 0 && flag_join == 0)
        fprintf(stdout, "no file created");
    if (strcmp(message, "delete") == 0 && flag_create > 0)
        handle_delete(arg1);
    if (strcmp(message, "get") == 0)
        handle_get(arg1, arg2);
    if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag_join == 1)
        handle_st();
    else if (((strcmp(message, "show") == 0 && strcmp(arg1, "topology") == 0) || strcmp(message, "st") == 0) && flag_join == 0)
        fprintf(stdout, "no node created\n");
    if (strcmp(message, "show names") == 0 || strcmp(message, "sn") == 0)
        handle_sn();
    if (strcmp(message, "show routing") == 0 || strcmp(message, "sr") == 0)
        handle_sr(arg2);
    if (strcmp(message, "exit") == 0)
    {
        close(server.my_node.fd);
        fprintf(stdout, "exiting program\n");
        exit(1);
    }
    if (strcmp(message, "clear") == 0)
    {
        clear(arg1);
        exit(1);
    }
    return rfds_list;
}

fd_set client_fd_set(fd_set rfds_list, int x)
{
    char buff[1024] = "", str_temp[10];
    char message[50], response[6];
    node_t temp;
    memset(buff, 0, 1024);
    int save = server.vz[x].fd;
    int intr = 0, i;

    for (intr = 1; intr < MAX_NODES; intr++)
    {
        if (server.vz[intr].fd != -1)
            break;
    }

    if (read(server.vz[x].fd, buff, 1024) == 0)
    {
        printf("%s\n",server.vz[x].id);
        withdraw(atoi(server.vz[x].id));
        
        close(server.vz[x].fd);
        if (x > 0)
        {
            server.vz[x].fd = -1;
        }
        else if (strcmp(server.my_node.id, server.vb.id) != 0) // VE saiu e nao é ancora, tem VI
        {
            server.vz[x] = server.vb;
            server.vz[x].fd = tcp_client(server.vb.ip, atoi(server.vb.port));

            sprintf(message, "NEW %s %s %s\n", server.my_node.id, server.my_node.ip, server.my_node.port);
            write(server.vz[x].fd, message, strlen(message));

            sprintf(message, "EXTERN %s %s %s\n", server.vz[x].id, server.vz[x].ip, server.vz[x].port);
            // for loop a enviar EXTERN aos intr
            for (i = 1; i < MAX_NODES; i++)
            {
                if (server.vz[i].fd != -1)
                {
                    write(server.vz[i].fd, message, strlen(message));
                }
            }
        }
        else if (intr != MAX_NODES)
        {
            server.vz[x] = server.vz[intr];

            sprintf(message, "EXTERN %s %s %s\n", server.vz[x].id, server.vz[x].ip, server.vz[x].port);
            for (i = 1; i < MAX_NODES; i++)
            {
                if (server.vz[i].fd != -1)
                {
                    write(server.vz[i].fd, message, strlen(message));
                }
            }
            server.vz[intr].fd = -1;
        }
        else
        {
            server.vz[x] = server.my_node;
            server.vz[x].fd = -1;
        }

    }
    else
    {
        fprintf(stdout, "recebi-%s", buff);
        sscanf(buff, "%s %s %s %s", str_temp, temp.id, temp.ip, temp.port);
        if (strcmp(str_temp, "NEW") == 0)
        {
            server.vz[x] = temp;
            server.vz[x].fd = save;
            sprintf(buff, "EXTERN %s %s %s\n", server.vz[0].id, server.vz[0].ip, server.vz[0].port);
            write(server.vz[x].fd, buff, strlen(buff));
            exptable[atoi(temp.id)]=atoi(temp.id);
        }
        if (strcmp(str_temp, "EXTERN") == 0)
        {
            server.vb = temp;
            exptable[atoi(server.vz[x].id)]=atoi(server.vz[x].id);
            exptable[atoi(temp.id)]=atoi(server.vz[x].id);
        }
        if (strcmp(str_temp, "QUERY") == 0)
        {
            exptable[atoi(temp.ip)]=atoi(server.vz[x].id);
            int res=dad_get(temp.id,temp.port,temp.ip);
            if (res==1)
            {
                sprintf(buff, "CONTENT %s %s %s\n", temp.ip, temp.id, temp.port);
                write(server.vz[x].fd, buff, strlen(buff));

            }
            if (res==2)
            {
                printf("daqui------------------\n");
                sprintf(buff, "NOCONTENT %s %s %s\n", temp.ip, temp.id, temp.port);
                write(server.vz[x].fd, buff, strlen(buff));
            }

        }
        if (strcmp(str_temp, "NOCONTENT")==0)
        {
            exptable[atoi(temp.ip)]=atoi(server.vz[x].id);
            if (strcmp(temp.id,server.my_node.id)!=0)
            {
                int temp_ip=exptable[atoi(temp.id)];
                for (int i = 0; i < 99; i++)
                {
                    if(atoi(server.vz[i].id)==temp_ip)
                    {
                        printf("daqui???asasdasdsadads?\n");
                        sprintf(buff, "NOCONTENT %s %s %s\n", temp.id, temp.ip, temp.port);
                        write(server.vz[i].fd, buff, strlen(buff));
                    }
                }
            }
        }
        if (strcmp(str_temp, "CONTENT")==0)
        {
            exptable[atoi(temp.ip)]=atoi(server.vz[x].id);
            if (strcmp(temp.id,server.my_node.id)!=0)
            {
                int temp_ip=exptable[atoi(temp.id)];
                for (int i = 0; i < 99; i++)
                {
                    if(atoi(server.vz[i].id)==temp_ip)
                    {
                        sprintf(buff, "CONTENT %s %s %s\n", temp.id, temp.ip, temp.port);
                        write(server.vz[i].fd, buff, strlen(buff));
                    }
                }
            }
        }
        if (strcmp(str_temp, "WITHDRAW")==0)
        {
            withdraw(atoi(temp.id));
        }
    }

    return rfds_list;
}


int dad_get(char *dest, char *name,char* origem)
{

    if (strcmp(dest,server.my_node.id)==0)
    {
        printf("tou no destino\n");
        for (int i = 0; i < 99; i++)
        {

            if (strcmp(name,server.names[i])==0)//tenho a info
            {
                return 1;
            }
            if (i==98)//n tenho a info
            {
                return 2;
            }
        }
    }
    char buff[255];
    int path=-1;
    int dest_int=atoi(dest);

    if(exptable[dest_int]!=0)//temos entrada na tabela de espedição
    {
        sprintf(buff, "QUERY %s %s %s \n", dest, origem, name);
        path = exptable[dest_int];
        for (int i = 0; i < 99; i++)
        {
            if(atoi(server.vz[i].id)==path)
            {
                write(server.vz[i].fd, buff, strlen(buff));
                break;
            }
        }
    }else
    {
        sprintf(buff, "QUERY %s %s %s\n", dest, origem, name);
        for (int i = 0; i < 99; i++)
        {
            if(server.vz[i].fd!=-1 && atoi(server.vz[i].id)!=atoi(origem))
            {
                write(server.vz[i].fd, buff, strlen(buff));
            } 
        }
    }

    return 0;

}

void withdraw(int x)
{
    char buff[100];
    char x_c[3]="";
    sprintf(x_c,"%02d",x);
    exptable[x]=0;   
    for (int i = 0; i < MAX_NODES; i++)
        {
            if(exptable[i]==x)
                exptable[i]=0;
            if(i!=x)
                {
                    sprintf(buff, "WITHDRAW %s\n",x_c);
                    write(server.vz[i].fd, buff, strlen(buff));
                }
        }
}
