// Written by Alexander Starr

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
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

// Frees memory which was allocated for reading from the socket.
void read_next_free(read_state_t *read_state) {
    free(read_state->buffer);
    free(read_state);
}

// Returns the next character from the socket, and handles any errors.
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
/*
// Will handle getting address info, and will have the result of
// servinfo now pointing to a linked list of results
// Didn't end up using this, but it would be useful for external IP addresses.
void getaddrinfo_w(char *port_no, struct addrinfo *servinfo) {
    printf("In getaddrinfo_w\n");
    int status;
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints); // Clear the struct.
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, port_no, &hints, &servinfo)) != 0) {
        perror("getaddrinf failed");
        exit(1);
    } else {
        printf("getaddrinf returned no errors.\n");
    }
}

// Prints the relevant information from an addrinfo struct.
// Didn't end up using this, but it would be useful for external IP adresses.
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
}*/

// Returns a socket file descriptor, or exits on error.
int socket_w(int domain, int type, int protocol) {
    int sock = socket(domain, type, protocol);
    if (sock < 0) {
        perror("socket failed");
        exit(1);
    } else {
        //printf("The socket is: %d\n", sock);
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
        perror("bind failed");
        exit(1);
    }
    //printf("Socket is bound\n");
}

// Sets a socket to listen and handles any errors.
void listen_w(int sock) {
    if (listen(sock, 1) < 0) {
        perror("listen failed");
        exit(1);
    }
    //printf("Socket is listening\n");
}

// Input:
// socket - the client socket from where data will be read
// uri - a pointer to where the uri will be saved
// Output:
// returns 0 if there was no error and -1 if an error has occured
int process_http_header(int socket, char *uri) {
    // Set up variables
    int error = 0;
    int i = 6;
    char ch;
    jmp_buf error_handler;
    // Initialize memory to be ready to read from the socket.
    read_state_t* read_state = read_next_init();
    // Set the point to jump to on error.
    // If an error is encountered, longjmp will jump to this spot, and set
    // the `error` variable to the error code so it can be handled appropriately.
    error = setjmp(error_handler);
    if (error >= 0) {
        // If we have no errors, then read the request.
        // Request should be in the following format:
        // "GET /[path_to_resource] HTTP/1.1\r\n(key: value\r\n)*\r\n"
        if (read_next_char(socket, read_state, error_handler) != 'G') return -1;
        if (read_next_char(socket, read_state, error_handler) != 'E') return -1;
        if (read_next_char(socket, read_state, error_handler) != 'T') return -1;
        if (read_next_char(socket, read_state, error_handler) != ' ') return -1;
        ch = read_next_char(socket, read_state, error_handler);

        // Fill the uri buffer with the address (it's prepended with "static").
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
        
        // At this point, the first line (the most important) is correct.
        // Now just scan until finding the sequence "\r\n\r\n" to know that
        // the rest of the request is in the correct (enough) format.
        ch = read_next_char(socket, read_state, error_handler);
        while (1) {
            while (ch != '\r') {
                // Scan until the end of the line.
                ch = read_next_char(socket, read_state, error_handler);
            }
            // See if this is the last line (ends with "\r\n\r\n")
            ch = read_next_char(socket, read_state, error_handler);
            if (ch == '\n') {
                ch = read_next_char(socket, read_state, error_handler);
                if (ch == '\r') {
                    ch = read_next_char(socket, read_state, error_handler);
                    if (ch == '\n') {
                        // Exit with the success code if this sequence is found.
                        return 0;
                    }
                }
            }
            // Otherwise keep scanning the next line.
            // If the sequence "\r\n\r\n" does not exist, then we will eventually
            // reach the end of the stream and trigger ERROR_END_OF_STREAM.
            // This is handled by the error_handler.
        }
    } else {
        switch(error) {
            case ERROR_END_OF_STREAM:
                // If the end of the stream is encountered, then that means
                // we never found the "\r\n\r\n" sequence, and the request
                // must have been formatted improperly.
                return -1;
            case ERROR_WHILE_RECEIVING:
                perror("error while receiving");
                exit(1);
        }
    }
    // Free up the memory.
    read_next_free(read_state);
    //printf("Request address: %s\n", uri);
    return 0;
}

// Sends the specified header
// 0 for '200 OK', 1 for '400 Bad Request', 2 for '404 Not Found'
void send_header(int client_sock, int head_type) {
    // Initialize the header strings and other variables.
    char ok_head[45] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    char bad_head[29] = "HTTP/1.1 400 Bad Request\r\n\r\n";
    char notfound_head[27] = "HTTP/1.1 404 Not Found\r\n\r\n";
    char *header;
    int len, bytes_sent;
    // Assign the proper header string based on the head_type.
    if (head_type == 0) {
        header = ok_head;
    } else if (head_type == 1) {
        header = bad_head;
    } else if (head_type == 2) {
        header = notfound_head;
    }
    // Then send the header string to the socket.
    len = strlen(header);
    //printf("Header length = %d\n", len);
    bytes_sent = send(client_sock, header, len, 0);
    //printf("Bytes sent = %d\n", bytes_sent);
    if (bytes_sent < 0) {
        perror("header send failed");
        exit(1);
    }
}

// Sends the text of the given file to the socket.
// This assumes that the file exists and the file pointer is valid.
void send_body(int client_sock, FILE *fp) {
    struct stat file_stats;
    char *line;
    int len, bytes_sent, fd, file_size;
    // Get the size of the file.
    fd = fileno(fp);
    if (fstat(fd, &file_stats) < 0) {
        perror("couldn't get file stats");
        exit(1);
    }
    file_size = file_stats.st_size;
    //printf("File size is: %d\n", file_size);
    // Allocate an array which can hold the whole file.
    // Thus it can hold the largest line in the file.
    line = (char*) malloc(file_size*sizeof(char) + 1);
    if (line == NULL) {
        perror("memory allocation failed");
        exit(1);
    }
    // While there are still lines in the file, send them to the socket.
    while (fgets(line, MAX_LEN, fp) != NULL) {
        len = strlen(line);
        bytes_sent = send(client_sock, line, len, 0);
        if (bytes_sent < 0) {
            perror("body send failed");
            exit(1);
        }
    }
    // Free the allocated memory.
    free(line);
}

int main(int argc, char **argv) {
    //struct addrinfo *servinfo;
    int port_no, sock, client_sock, head_id;
    struct sockaddr_storage client_addr;
    char uri[URI_SIZE + 7] = "static";
    FILE *fp;
    // Get the port number, if it was given.
    if (argc != 2) {
        perror("no port provided");
        exit(1);
    } else {
        port_no = atoi(argv[1]);
    }
    //printf("The port number is: %d\n", port_no);

    // Get the address info struct
    /*printaddrinfo(servinfo);
    getaddrinfo_w(argv[1], servinfo);
    printaddrinfo(servinfo);*/

    // Set up the socket.
    sock = socket_w(PF_INET, SOCK_STREAM, 0);
    bind_w(sock, port_no);
    listen_w(sock);
    printf("Server is ready on port %d\n", port_no);
    while(1) {
        // Accept client requests to the socket already created.
        // Create a socket for each client.
        socklen_t addr_size = sizeof client_addr;
        if ((client_sock = accept(sock, (struct sockaddr *) &client_addr, &addr_size)) < 0) {
            perror("accept failed");
            exit(1);
        }
        //printf("New client socket no: %d\n", client_sock);

        // Process the request header.
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
        // Send the appropriate header.
        send_header(client_sock, head_id);

        // The request is good and the file exists, so send the file.
        if (head_id == 0) send_body(client_sock, fp);

        // Then reset the uri buffer and close the connection to the client.
        memset(uri, 0, sizeof uri);
        sprintf(uri, "static");
        close(client_sock);
    }
    //freeaddrinfo(servinfo);
    return 0;
}
