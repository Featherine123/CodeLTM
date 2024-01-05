#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#define SERV_PORT 5550
#define BUFF_SIZE 1024

int checkValid(const char *input) {
    for (int i = 0; input[i] != '\0'; i++) {
        if (!isalnum(input[i])) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Use:  ./client IPAddress PortNumber\n");
        return 1;
    }

    char* ip = argv[1];
    char* port = argv[2];
    int port_number = atoi(port);

    int client_sock;
    char buff[BUFF_SIZE];
    size_t bytes_sent, bytes_received;
    struct sockaddr_in server_addr;

    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Step 3: Request to connect to the server
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0) {
        perror("Error connecting to server");
        close(client_sock);
        return 1;
    }

    do {
        // Step 4: Receive data from the server
        bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("Error receiving data");
            close(client_sock);
            return 1;
        }
        buff[bytes_received] = '\0';
        printf("Reply from server: %s\n", buff);

        // Clear the buffer before taking new input
        memset(buff, '\0', sizeof(buff));

        // Step 5: Input string to send
        printf("Insert string to send (or @ or # to exit): ");
        fgets(buff, BUFF_SIZE, stdin);

        // Remove the newline character from the input
        buff[strcspn(buff, "\n")] = '\0';

        if (strlen(buff) == 0 || strcmp(buff, "@") == 0 || strcmp(buff, "#") == 0) {
            // exit if the input string is empty or equal to "@" or "#"
            break;
        }

        // Step 6: Send data to the server
        bytes_sent = send(client_sock, buff, BUFF_SIZE, 0);
        if (bytes_sent < 0) {
            perror("Error sending data");
            close(client_sock);
            return 1;
        }

    } while (1);

    // Step 7: Close the socket
    close(client_sock);

    return 0;
}
