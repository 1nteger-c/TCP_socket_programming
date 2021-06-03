#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

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
    client->sin_family = AF_INET;   // define domain as IPv4 by using "AF_INET"
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
    if (connect(client_s, (struct sockaddr *)client, sizeof(sock)) == -1)   // if error occured, connect() return -1
    {
        printf("<ERROR> error on connecting sokect <ERROR>");
        close(client_s);    // we must close the socket and finish the program
        free(client);
        return -1;
    }
    char message[2][30] = {"READY: You are a sender.", "READY: You are a receiver."};   // the strings that server will send
    char *msg = (char *)malloc(1030);   // msg : the buffer of the message that will be send and receive
    memset(msg, '\x00', 1030);  // set the memory to '\x00' becuase of the concern of junk data.
    recv(client_s, msg, 30, 0); // receive message from server, and by this message, client can determine one's role
    printf("%s\n", msg); // print the received message
    if (!strcmp(msg, message[0]))   // the case the client is sender
    {
        /*
        set the memory to null
        and send the data to server
        Actually, that data will be sended to receiver client
        */
        memset(msg, '\x00', 1030);  // set the memory 
        fgets(msg, 1000, stdin);
        send(client_s, msg, strlen(msg), 0);
    }
    else if (!strcmp(msg, message[1]))  // the case the client is receiver
    {
        /*
        set the memory to null
        and recv the data from server
        Actually, that data is sended from sender client
        and print the data
        */
        memset(msg, '\x00', 1030);
        recv(client_s, msg, 1000, 0);
        printf("%s", msg);
    }
    else
    {
        /*
        it must not occured but if it will, it means that some error occured
        close the socket, and terminate the program
        */
        printf("<ERROR> error on exchanging data from server <ERROR>");
        close(client_s);
        free(client);
        free(msg);
        return -1;
    }
    close(client_s);    // the progress is done. Let's close the socket and terminate the program
    free(client);
    free(msg);
    return 0;
}
