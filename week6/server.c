#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 5500
#define BUFF_SIZE 1024

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    char buffer[BUFF_SIZE];

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr) == -1)) {
        perror("Binding failed");
        exit(1);
    }

    if (listen(server_sock, 5) == -1) {
        perror("Listening failed");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);

    // Receive and save the image sent by the client
    FILE *imageFile = fopen("received_image.jpg", "wb");
    if (imageFile == NULL) {
        perror("File open failed");
        exit(1);
    }

    while (1) {
        int bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) break;

        fwrite(buffer, 1, bytes_received, imageFile);
    }

    fclose(imageFile);
    close(client_sock);
    close(server_sock);

    printf("Image received and saved as 'received_image.jpg'\n");

    return 0;
}
