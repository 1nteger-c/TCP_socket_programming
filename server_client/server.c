#include <stdio.h>
#include <sys/socket.h> // contain socket, bind, listen, accept, send, recv
#include <stdlib.h>     // contain atoi, malloc
#include <netinet/in.h> // contain struct sockaddr_in
#include <unistd.h>  // close
#include <string.h> // contain memset

typedef struct sockaddr_in sock; // define "sock" as "struct sockaddr_in"
void usage()
{
    printf("Input Format : ./server [port]"); // print how to use this program
}
int main(int argc, char **argv)
{
    if (argc != 2) // if the parameter format is not available 
    {
        usage();
        return -1;
    }
    /*
    open socket
    domain : "AF_INET" : use IPv4
    type : "SOCK_STREAM" : use TCP
    */
    int server_s = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_s == -1) // if error on opening socket, socket() return -1 
    {
        printf("<ERROR> error on opening sokect <ERROR>");
        return -1;
    }
    sock *server = (sock *)malloc(sizeof(sock));    // make sock to use in "binding"'
    /*  
    if there is some junk data, it would make some error
    so set the memory as '\x00' (NULL)    
    */
    memset(server, '\x00', sizeof(sock));           
    server->sin_family = AF_INET;   // define domain as IPv4 by using "AF_INET"
    /*
    set server's IP automatically by using htonl(INADDR_ANY)
    */
    server->sin_addr.s_addr = htonl(INADDR_ANY);
//    inet_aton(argv[1], &(server->sin_addr));        
    /*
    we got server's PORT by parameter 1 (argv[1])
    we can change PORT, ascii to int by using htons
    */
    server->sin_port = htons(atoi(argv[1]));        
    /*
    we must bind socket and process. 
    Then process can use socket to communicate with others.
    */
    if (bind(server_s, (struct sockaddr *)server, sizeof(sock)) == -1)
    {
        printf("<ERROR> error on binding sokect <ERROR>");
        close(server_s);    // we must close the socket and finish the program
        free(server);
        return -1;
    }
    /*
    function :listen(int fd, int backlog_queue_size)
    we will connect 2 client in this server.
    so put 2 in "backlog_queue_size"
    and socket_fd in "fd"
    */
    if (listen(server_s, 2) == -1) // if error occured, listen() return -1
    {
        printf("<ERROR> error on listening <ERROR>");
        close(server_s);    // we must close the socket and finish the program
        free(server);
        return -1;
    }
    /*
    make pointer of sock which will be used for client side
    use "malloc" to allocate memory
    and set the memory to '\x00' becuase of the concern of junk data.
    */
    sock *client;
    client = (sock *)malloc(sizeof(sock));
    memset(client, '\x00', sizeof(sock));
    /*
    client_s[0] : will be a sender
    client_s[1] : will be a receiver
    */
    int client_s[2];
    unsigned int cl_size = sizeof(sock); // accept's third parameter is &size, so make the variable to use.
    char message[2][30] = {"READY: You are a sender.", "READY: You are a receiver."}; // will be sent to sender and receiver each.
    for (int i = 0; i < 2; i++)
    {
        client_s[i] = accept(server_s, (struct sockaddr *)client, &cl_size); // client connect to server and server accepted it.
        if(client_s[i] == -1){ // if the error occured, we should close the sockets and terminate the program
            close(server_s);
            for(int j=0;j<i;j++)
                close(client_s[j]);
            free(server);
            return -1;
        }
    }
    for(int i=0;i<2;i++){
        send(client_s[i], message[i], strlen(message[i]), 0); // send the server's message to client to determine the role of each client
    }
    char buf[1030]; // used for client's data buffer memory
    /*
    recv the data from sender by using "recv"
    and relay it to receiver by using "send"
    */
    if (recv(client_s[0], buf, 1000, 0) == -1) // if error occured, recv() return -1
    {
        printf("<ERROR> error on recving data on receiving data from sender <ERROR>");
        close(server_s); // we must close the socket and finish the program
        close(client_s[0]);
        close(client_s[1]);
        free(server);
        return -1;
    }
    if (send(client_s[1], buf, strlen(buf), 0) == -1) // if error occured, send() return -1
    {
        printf("<ERROR> error on sending data on sending data to receiver <ERROR>");
        close(server_s); // we must close the socket and finish the program
        close(client_s[0]);
        close(client_s[1]);
        free(server);
        return -1;
    }
    close(server_s); // the progress of sending data from sender to receiver is done! Let's close the socket and terminate the program
    close(client_s[0]);
    close(client_s[1]);
    free(server);
    return 0;
}
