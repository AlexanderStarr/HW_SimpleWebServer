#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <setjmp.h>

#define READ_STATE_BUFFER 32
#define ERROR_END_OF_STREAM -1
#define ERROR_WHILE_RECEIVING -2
#define URI_SIZE 255
#define MAX_LEN 1024

// Struct used for reading in info from the socket.
typedef struct {
    char *buffer;
    int size;
    int index;
} read_state_t;

// Initializes the read_state_t struct for reading from the socket.
read_state_t* read_next_init() {
    read_state_t *read_state = (read_state_t *) malloc(sizeof(read_state_t));
    read_state->buffer = (char *) malloc(sizeof(char) * READ_STATE_BUFFER);
    read_state->size = 0;
    read_state->index = 0;
    return read_state;
}

// Frees allocated memory from reading from the socket.
void read_next_free(read_state_t *read_state) {
    free(read_state->buffer);
    free(read_state);
}

char read_next_char(int socket, read_state_t *read_state, jmp_buf error_handler) {
    char ch;

    if (read_state->index < read_state->size) {
        ch = read_state->buffer[read_state->index];
        read_state->index = read_state->index + 1;
        return ch;
    }

    read_state->size = recv(socket, read_state->buffer, READ_STATE_BUFFER, 0);
    if (read_state->size == 0) {
        // End of stream.
        longjmp(error_handler, ERROR_END_OF_STREAM);
    } else if (read_state->size < 0) {
        // Error occurred.
        longjmp(error_handler, ERROR_WHILE_RECEIVING);
    }

    ch = read_state->buffer[0];
    read_state->index = 1;
    return ch;
}

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

// Input:
// socket - the client socket from where data will be read
// uri - a pointer to where the uri will be saved
// Output:
// returns 0 if there was no error and -1 if an error has occured
int process_http_header(int socket, char *uri) {
    int error = 0;
    int i = 6;
    char ch;
    jmp_buf error_handler;
    read_state_t* read_state = read_next_init();
    error = setjmp(error_handler);
    if (error >= 0) {
        if (read_next_char(socket, read_state, error_handler) != 'G') return -1;
        if (read_next_char(socket, read_state, error_handler) != 'E') return -1;
        if (read_next_char(socket, read_state, error_handler) != 'T') return -1;
        if (read_next_char(socket, read_state, error_handler) != ' ') return -1;
        ch = read_next_char(socket, read_state, error_handler);
        // Fill the uri buffer up with the address (it's prepended with 'static')
        while (ch != ' ') {
            uri[i] = ch;
            i += 1;
            ch = read_next_char(socket, read_state, error_handler);
        }
        if (read_next_char(socket, read_state, error_handler) != 'H') return -1;
        if (read_next_char(socket, read_state, error_handler) != 'T') return -1;
        if (read_next_char(socket, read_state, error_handler) != 'T') return -1;
        if (read_next_char(socket, read_state, error_handler) != 'P') return -1;
        if (read_next_char(socket, read_state, error_handler) != '/') return -1;
        if (read_next_char(socket, read_state, error_handler) != '1') return -1;
        if (read_next_char(socket, read_state, error_handler) != '.') return -1;
        if (read_next_char(socket, read_state, error_handler) != '1') return -1;
        ch = read_next_char(socket, read_state, error_handler);
        while (1) {
            while (ch != '\r') {
                ch = read_next_char(socket, read_state, error_handler);
            }
            ch = read_next_char(socket, read_state, error_handler);
            if (ch == '\n') {
                ch = read_next_char(socket, read_state, error_handler);
                if (ch == '\r') {
                    ch = read_next_char(socket, read_state, error_handler);
                    if (ch == '\n') {
                        return 0;
                    }
                }
            }
        }
    } else {
        switch(error) {
            case ERROR_END_OF_STREAM:
                return -1;
            case ERROR_WHILE_RECEIVING:
                exit(1);
        }
    }
    read_next_free(read_state);
    printf("Request address: %s\n", uri);
    return 0;
}

// Sends the specified header
// 0 for '200 OK', 1 for '400 Bad Request', 2 for '404 Not Found'
void send_header(int client_sock, int head_type) {
    char ok_head[45] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    char bad_head[29] = "HTTP/1.1 400 Bad Request\r\n\r\n";
    char notfound_head[27] = "HTTP/1.1 404 Not Found\r\n\r\n";
    char *header;
    int len, bytes_sent;
    if (head_type == 0) {
        header = ok_head;
    } else if (head_type == 1) {
        header = bad_head;
    } else if (head_type == 2) {
        header = notfound_head;
    }
    len = strlen(header);
    printf("Header length = %d\n", len);
    bytes_sent = send(client_sock, header, len, 0);
    printf("Bytes sent = %d\n", bytes_sent);
    if (bytes_sent < 0) {
        exit(1);
    }
}

void send_body(int client_sock, FILE *fp) {
    char line[MAX_LEN];
    int len, bytes_sent;
    while (fgets(line, MAX_LEN, fp) != NULL) {
        len = strlen(line);
        bytes_sent = send(client_sock, line, len, 0);
    }
}

// Sends the given html file.

int main(int argc, char **argv) {
    // Get the port number, if it was given.
    struct addrinfo *servinfo;
    int port_no, sock, client_sock, head_id;
    struct sockaddr_storage client_addr;
    char uri[URI_SIZE + 7] = "static";
    FILE *fp;
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
        if (process_http_header(client_sock, uri) == -1) {
            // We want a 'Bad request' response if the http_header is invalid.
            head_id = 1;
        } else {
            // Otherwise we will look for the file.
            fp = fopen(uri, "r");
            if (fp==NULL) {
                // If not found, then we will return a 'Not found' response.
                head_id = 2;
            } else {
                // Otherwise, we will send the header to prepend the file.
                head_id = 0;
            }
        }
        send_header(client_sock, head_id);
        if (head_id == 0) send_body(client_sock, fp);
        memset(uri, 0, sizeof uri);
        sprintf(uri, "static");
        close(client_sock);
    }
    freeaddrinfo(servinfo);
    return 0;
}
