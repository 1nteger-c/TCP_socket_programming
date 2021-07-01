#include <stdio.h>
#include <sys/socket.h> // contain socket, bind, listen, accept, send, recv
#include <stdlib.h>     // contain atoi, malloc
#include <netinet/in.h> // contain struct sockaddr_in
#include <unistd.h>     // close
#include <string.h>     // contain memset

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

    int server_s = socket(AF_INET, SOCK_STREAM, 0);
    if (server_s == -1) // if error on opening socket, socket() return -1
    {
        printf("<ERROR> error on opening sokect <ERROR>");
        return -1;
    }
    sock *server = (sock *)malloc(sizeof(sock)); // make sock to use in "binding"'
    /*  
    if there is some junk data, it would make some error
    so set the memory as '\x00' (NULL)    
    */
    memset(server, '\x00', sizeof(sock));
    server->sin_family = AF_INET; // define domain as IPv4 by using "AF_INET"
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
        close(server_s); // we must close the socket and finish the program
        free(server);
        return -1;
    }
    /*
    function :listen(int fd, int backlog_queue_size)
    we will connect 2 client in this server.
    so put 2 in "backlog_queue_size"
    and socket_fd in "fd"
    */
    if (listen(server_s, 5) == -1) // if error occured, listen() return -1
    {
        printf("<ERROR> error on listening <ERROR>");
        close(server_s); // we must close the socket and finish the program
        free(server);
        return -1;
    }

    fd_set reads, cp_reads;
    int fd_max;
    struct timeval time_out;


    FD_ZERO(&reads);          // initiate fd_set variable
    FD_SET(server_s, &reads); // set the server fd's bit to 1 => will check on select function
    fd_max = server_s;        // we should use it on select function's first parameter

    int fd_num;
    socklen_t cl_size = sizeof(sock);

    /*
    make pointer of sock which will be used for client side
    use "malloc" to allocate memory
    and set the memory to '\x00' becuase of the concern of junk data.
    */
    sock *client;
    client = (sock *)malloc(sizeof(sock));
    memset(client, '\x00', sizeof(sock));
    int client_s;

    size_t msg_len;
    char msg[1030]; // it will be used as buffer
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
            for (int i = 0; i <= fd_max; i++)
            {
                if (FD_ISSET(i, &cp_reads)) // check if "i" is fd that change occurred
                {
                    if (i == server_s) // the case that there is request for server. it should be connect request
                    {
                        client_s = accept(server_s, (struct sockaddr *)client, &cl_size); // accept the connection for client
                        FD_SET(client_s, &reads);                                         // use FD_SET to check the client's fd in select function
                        if (fd_max < client_s)                                            // we need to keep the fd_max variable as max value
                            fd_max = client_s;
                        printf("New client connected!! fd : %d\n", client_s);
                    }
                    else // the case that there is some input request for client
                    {
                        msg_len = recv(i, msg, 1000, 0); // receive the data
                        if (msg_len == 0)                // the case that there is "close" request
                        {
                            FD_CLR(i, &reads); // set the bit 0 by using "FD_CLR"
                            close(i);          // then close the client's fd
                            printf("client closed!! fd : %d\n", i);
                        }
                        else
                        {
                            send(i, msg, msg_len, 0); // relay the data to client
                        }
                    }
                }
            }
        }
    }
    // we should close the fd and free the memory before termination
    close(server_s);
    free(server);
    free(client);
    return 0;
}
