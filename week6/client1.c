#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SERVER_IP "127.0.0.1"  // Server IP address
#define PORT 5500
#define BUFF_SIZE 1024

int main() {
    int client_sock;
    struct sockaddr_in server_addr;
    char buffer[BUFF_SIZE];

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(1);
    }

    FILE *imageFile = fopen("image.jpg", "rb");
    if (imageFile == NULL) {
        perror("File open failed");
        exit(1);
    }

    while (1) {
        int bytes_read = fread(buffer, 1, sizeof(buffer), imageFile);
        if (bytes_read <= 0) break;

        send(client_sock, buffer, bytes_read, 0);
    }

    fclose(imageFile);
    close(client_sock);

    printf("Image sent to the server\n");

    return 0;
}
