#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

// Make sure that you check for error codes!  This is best done by using a wrapper
// function around things (like socket, listen, call, connect).
// Appropriate actions for any errors are to exit.
// send and recv should be more intelligent with handling errors.

int main(int argc, char **argv) {
    int port_no = -1;
    if (argc != 2) {
        exit(1);
    } else {
        port_no = atoi(argv[1]);
    }
    printf("The port number is: %d\n", port_no);
    return 0;
}
