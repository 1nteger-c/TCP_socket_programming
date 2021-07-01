#include <stdio.h>
#include <sys/socket.h> // contain socket, bind, listen, accept, send, recv
#include <stdlib.h>     // contain atoi, malloc
#include <arpa/inet.h>  // contain inet_aton
#include <netinet/in.h> // contain struct sockaddr_in
#include <unistd.h>     // close
#include <string.h>     // contain memset

typedef struct sockaddr_in sock; // define "sock" as "struct sockaddr_in"

void usage()
{
    printf("Input Format : ./client [IP] [port]"); // print how to use this program
}
int main(int argc, char **argv)
{
    if (argc != 3) // if the parameter format is not available
    {
        usage();
        return -1;
    }
    /*
    open socket
    domain : "AF_INET" : use IPv4
    type : "SOCK_STREAM" : use TCP
    */
    int client_s = socket(AF_INET, SOCK_STREAM, 0);
    if (client_s == -1) // if error on opening socket, socket() return -1
    {
        printf("<ERROR> error on opening sokect <ERROR>");
        return -1;
    }
    sock *client = (sock *)malloc(sizeof(sock)); // make sock to use in "connect"
    /*  
    if there is some junk data, it would make some error
    so set the memory as '\x00' (NULL)    
    */
    memset(client, '\x00', sizeof(sock));
    client->sin_family = AF_INET; // define domain as IPv4 by using "AF_INET"
    /*
    we got server's IP address by parameter 1 (argv[1])
    we can change IP, ascii to numeric by using inet_aton
    */
    inet_aton(argv[1], &(client->sin_addr));
    /*
    we got server's PORT by parameter 2 (argv[2])
    we can change PORT, ascii to int by using htons
    */
    client->sin_port = htons(atoi(argv[2]));
    /*
    we must request the connection to socket
    */

    if (connect(client_s, (struct sockaddr *)client, sizeof(sock)) == -1) // if error occured, connect() return -1
    {
        printf("<ERROR> error on connecting sokect <ERROR>");
        close(client_s); // we must close the socket and finish the program
        free(client);
        return -1;
    }

    fd_set reads, cp_reads;
    int fd_max;
    struct timeval time_out;

    FD_ZERO(&reads);          // initiate fd_set variable
    FD_SET(0, &reads);        // set the "Standard Input" fd's bit to 1 => will check on select function
    FD_SET(client_s, &reads); // set the server fd's bit to 1 => will check on select function
    fd_max = client_s;        // we should use it on select function's first parameter

    int fd_num;

    char msg[1030]; // it will be used as buffer
    int recv_len;
    while (1)
    {
        cp_reads = reads;    // we must use it by copy varatble. because it will be changed in function "FD_ISSET"
        time_out.tv_sec = 3; // I set the timeout as 3 second.
        time_out.tv_usec = 3000;

        fd_num = select(fd_max + 1, &cp_reads, 0, 0, &time_out); // check that is there any input request on server fd
        if (fd_num == -1)                                        // if there is any error on selct function, it will return -1
        {
            break;
        }
        else if (fd_num == 0) // if timeout occurred, select function will return 0;
        {
            continue;
        }
        /* 
        if the return value of "select" isn't 0 or -1, there must be some request
        Actually We don't know exact value of fd which some change occurred, so we use "for" to check all fd less than fd_max
        By using "FD_ISSET", we can check that where is some change.
        */
        else
        {
            if (FD_ISSET(0, &cp_reads)) // the case that there is some standard input 
            {
                memset(msg, '\x00', 1030); // set the msg buffer as null value
                fgets(msg, 1000, stdin); // get the standard input 
                recv_len = strlen(msg);
                if (msg[recv_len - 1] == '\n') // if last byte is "enter", I changed it to null value.
                    msg[recv_len - 1] = '\x00';
                send(client_s, msg, strlen(msg), 0); // send the message to the server
            }
            if (FD_ISSET(client_s, &cp_reads)) // the case that there is some request for client which should be message relay of server
            {
                memset(msg, '\x00', 1030);
                recv_len = recv(client_s, msg, 1000, 0); // receive the msg 
                printf("received data : %s\n", msg); // print the message which process get
            }
        }
    }
    // we should close the fd and free the memory before termination
    close(client_s);
    free(client);
    return 0;
}
