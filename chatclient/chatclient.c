#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "util.h"

///I pledge my honor that I have abided by the Stevens Honor System. -Isabella Stone

int client_socket = -1;
char username[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];

int handle_stdin() { 
    //if message is too long
    if (get_string(outbuf, sizeof(outbuf)) == TOO_LONG) {
        fprintf(stderr, "Sorry, limit your message to %d characters.\n", MAX_MSG_LEN);
        return EXIT_FAILURE;
    }    

    //else send message
    send(client_socket, outbuf, sizeof(outbuf), 0);

    //if want you want to end connection
    if(strcmp(outbuf, "bye") == 0) {
        close(client_socket);
        return 50;
    }

    return 0;
}

int handle_client_socket() {
    memset(&inbuf, 0, sizeof(inbuf));
    int recbytes = recv(client_socket, inbuf, sizeof(inbuf), 0);

    //if bytes are invalid
    if (recbytes == -1) {
        printf("Warning: Failed to receive incoming message. %s.\n", strerror(errno));
        return 100; //to continue
    }
    else if (recbytes == 0) {
        fprintf(stderr, "Connection to server has been lost.\n");
        return EXIT_FAILURE;
    }

    //null-terminate inbuf and compare to bye, else print message
    inbuf[recbytes] = 0;
    if (strcmp(inbuf, "bye") == 0) {
        printf("\nServer initiated shutdown.\n");
        return 50;
    }
    else {
        printf("\n%s\n", inbuf);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    
    //if no arguments are supplied
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    //check if IP address is valid
    struct sockaddr_in server_addr;
    socklen_t server_addr_size = sizeof(server_addr);
    memset(&server_addr, 0, server_addr_size);
    int valid_IP = inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    if (valid_IP != 1) {
        fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
        return EXIT_FAILURE;
    }

    int i = 0;
    if (!parse_int(argv[2], &i, "port number")) {
        return EXIT_FAILURE;
    }

    if (i < 1024 || i > 65535) {
        fprintf(stderr, "Error: Port must be in range [1024, 65535].\n");
        return EXIT_FAILURE;
    }
   
    //get username
    while(1) {
        printf("Enter your username: ");
        fflush(stdout);
        int status = get_string(username, sizeof(username));

        if (status == TOO_LONG) {
            printf("Sorry, limit your username to %d characters.\n", MAX_NAME_LEN);
        } 
        else if (status == NO_INPUT) {
            continue;
        } 
        else {
            break;
        }
    }
    printf("Hello, %s. Let's try to connect to the server.\n", username);

    /////////////////////////////////////////server/client code/////////////////////////////////////////////////////////

    //create socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    //server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    
    //connect to server
    int connection = connect(client_socket, (struct sockaddr *) &server_addr, server_addr_size);
    if (connection == -1) {
        fprintf(stderr, "Error: Failed to connect to server. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    
    int recvbytes = recv(client_socket, inbuf, sizeof(inbuf), 0);
    if (recvbytes < 0) {
        fprintf(stderr, "Error: Failed to receive message from server. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if (recvbytes == 0) {
        //break;
        fprintf(stderr, "All connections are busy. Try again later.\n");
        return EXIT_FAILURE;
    }
    else {
        inbuf[recvbytes] = 0;
        printf("\n%s\n\n", inbuf); 
        fflush(stdout);
        int sent = send(client_socket, username, strlen(username), 0);
        if (sent == -1) {
            fprintf(stderr, "Error: Failed to send username to server. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }
    }
    
    //while loop to send messages back and forth
    while(1) {
        fd_set rfds;

        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        FD_SET(client_socket, &rfds);

        printf("[%s]: ", username);
        fflush(stdout);

        select(client_socket + 1, &rfds, NULL, NULL, NULL);

        if (FD_ISSET(0, &rfds)) {
            //if "bye"
            if (handle_stdin() == 50) { 
                printf("Goodbye.\n");
                return EXIT_SUCCESS;
            }
        }
        else if (FD_ISSET(client_socket, &rfds)) {
            //if its a warning continue or successful message, continue:
            int num = handle_client_socket();
            if (num == 100 || num == EXIT_SUCCESS) {
                continue;
            }
            //if "bye"
            else if (num == 50) {
                return EXIT_SUCCESS;
            }
            else {
                return EXIT_FAILURE;
            }
        }
    }

    close(client_socket);

    return EXIT_SUCCESS;
}

