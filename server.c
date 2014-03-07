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
    int port_no;
    if (argc != 2) {
        exit(1);
    } else {
        port_no = atoi(argv[1]);
    }
    printf("The port number is: %d\n", port_no);

    // Get the address info struct
    struct addrinfo *servinfo;
    getaddrinfo_w(argv[1], servinfo);
    printaddrinfo(servinfo);
    freeaddrinfo(servinfo);
    return 0;
}
