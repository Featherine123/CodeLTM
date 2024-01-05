#include <stdio.h>
#include <stdlib.h>  
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <pthread.h>

#define MAX_CLIENTS 5
#define MAX_LENGTH 80
#define MAX_MESSAGE_SIZE 1024
// Khai báo biến toàn cục
int serverSocket;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int clientSockets[MAX_CLIENTS];

// Function to broadcast a message to all connected clients
void broadcastMessage(char *message, int senderSocket) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clientSockets[i] > 0 && clientSockets[i] != senderSocket) {
            send(clientSockets[i], message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}
// Khai báo các hàm
void* handleClient(void* arg);
void sendData(int clientSocket, char* data, size_t size);



int main(int argc, char const *argv[]) {
  if (argc != 2){
	printf("Cần nhập đúng định dạng: ./server PortNumber\n");
	return 1;
  }
  char *port = argv[1];
  int port_number = atoi(port);

  // Khởi tạo socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        printf("Error creating socket");
        return 0;
    }

// Thiết lập cấu trúc địa chỉ của server
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_number);

    // Liên kết socket với địa chỉ và số hiệu cổng
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        printf("Error binding socket");
        return 0;
    }

    // Lắng nghe kết nối từ client
    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        printf("Error listening for connections");
        return 0;
    }

    printf("Server is listening on port %d...\n", port_number);
  //init_server(port_number);


    // Initialize client sockets
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clientSockets[i] = 0;
    }

    // ...

    while (1) {
        // Chấp nhận kết nối mới từ client
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

        // Add the new client socket to the array
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clientSockets[i] == 0) {
                clientSockets[i] = clientSocket;
                break;
            }
        }
        pthread_mutex_unlock(&mutex);

        // Tạo một luồng mới cho client vừa kết nối
        pthread_t clientThread;
        pthread_create(&clientThread, NULL, handleClient, (void *)&clientSocket);
    }

    // Đóng socket của server (Không bao giờ được chạy đến đây trong khi vòng lặp vô hạn)
    close(serverSocket);
    return 0;
  
}

// Function to handle a single client
void *handleClient(void *arg) {
    int clientSocket = *((int *)arg);
    char message[MAX_MESSAGE_SIZE];

    while (1) {
        int bytesRead = recv(clientSocket, message, sizeof(message), 0);
        if (bytesRead <= 0) {
            break;  // Client disconnected
        }

        message[bytesRead] = '\0';  // Null-terminate the received message
        printf("Received from client: %s", message);

        // Broadcast the message to all other clients
        broadcastMessage(message, clientSocket);
    }

    // Remove the disconnected client
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clientSockets[i] == clientSocket) {
            clientSockets[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    close(clientSocket);
    pthread_exit(NULL);
}

void sendData(int clientSocket, char* data, size_t size) {
    pthread_mutex_lock(&mutex); // Lock mutex để đảm bảo an toàn khi truy cập dữ liệu chung
    // Gửi dữ liệu đến client
    send(clientSocket, data, size, 0);
    pthread_mutex_unlock(&mutex); // Unlock mutex sau khi hoàn thành
}














































