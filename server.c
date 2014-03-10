#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Make sure that you check for error codes!  This is best done by using a wrapper
// function around things (like socket, listen, call, connect).
// Appropriate actions for any errors are to exit.
// send and recv should be more intelligent with handling errors.

// Will handle getting address info, and will have the result of
// servinfo now pointing to a linked list of results
void getaddrinfo_w(char *port_no, struct addrinfo *servinfo) {
    printf("In getaddrinfo_w\n");
    int status;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints); // Clear the struct.
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port_no, &hints, &servinfo)) != 0) {
        printf("getaddrinf returned error %d\n", status);
        exit(1);
    } else {
        printf("getaddrinf returned no errors.\n");
    }
}

void printaddrinfo(struct addrinfo *servinfo) {
    struct addrinfo *i;
    printf("Printing addrinfo\nField\tValue\n");
    for (i = servinfo; i !=NULL; i = i->ai_next) {
        printf("ai_flags\t%d\n", i->ai_flags);
        printf("ai_family\t%d\n", i->ai_family);
        printf("ai_socktype\t%d\n", i->ai_socktype);
        printf("ai_protocol\t%d\n", i->ai_protocol);
        printf("ai_canonname\t%s\n", i->ai_canonname);
        
    }
}

// Returns a socket file descriptor, or exits on error.
int socket_w(int domain, int type, int protocol) {
    int sock = socket(domain, type, protocol);
    if (sock < 0) {
        exit(1);
    } else {
        printf("The socket is: %d\n", sock);
        return sock;
    }
}

// Binds the socket to the specified port on the loopback IP address.
void bind_w(int sock, int port_no) {
    struct sockaddr_in listenAddr;
    memset(&listenAddr, 0, sizeof(listenAddr));
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_port = htons(port_no);
    listenAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sock, (struct sockaddr *) &listenAddr, sizeof(listenAddr)) < 0) {
        exit(1);
    }
    printf("Socket is bound\n");
}

// Sets a socket to listen and handles any errors.
void listen_w(int sock) {
    if (listen(sock, 1) < 0) {
        exit(1);
    }
    printf("Socket is listening\n");
}

// 

// Input:
// socket - the client socket from where data will be read
// uri - a pointer to where the uri will be saved
// Output:
// returns 0 if there was no error and -1 if an error has occured
int process_http_header(int socket, char *uri) {
    return 0;
}

int main(int argc, char **argv) {
    // Get the port number, if it was given.
    struct addrinfo *servinfo;
    int port_no, sock, client_sock;
    struct sockaddr_storage client_addr;
    if (argc != 2) {
        exit(1);
    } else {
        port_no = atoi(argv[1]);
    }
    printf("The port number is: %d\n", port_no);

    // Get the address info struct
    printaddrinfo(servinfo);
    getaddrinfo_w(argv[1], servinfo);
    printaddrinfo(servinfo);
    sock = socket_w(PF_INET, SOCK_STREAM, 0);
    bind_w(sock, port_no);
    listen_w(sock);
    while(1) {
        socklen_t addr_size = sizeof client_addr;
        if ((client_sock = accept(sock, (struct sockaddr *) &client_addr, &addr_size)) < 0) {
            exit(1);
        }
        printf("New client socket no: %d\n", client_sock);
    }
    freeaddrinfo(servinfo);
    return 0;
}
