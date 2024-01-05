/*UDP Echo Client*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SERV_PORT 5550
#define SERV_IP "127.0.0.1"
#define BUFF_SIZE 1024

// Function to create a UDP socket
int createSocket() {
    int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_sock < 0) {
        perror("\nError: ");
        return -1;
    }
    return client_sock;
}
int checkValid(char* input){
    for(int i = 0; i < strlen(input)-1; i++){
        if(input[i] >= '0' && input[i] <= '9'){
            continue;
        }
        else if(input[i] >= 'a' && input[i] <= 'z'){
            continue;
        }
        else if(input[i] >= 'A' && input[i] <= 'Z'){
            continue;
        }
        else {
            return 0;
        }
    }
    return 1;
}
// Function to send a message to the server
int sendMessage(int client_sock, const char* message, const struct sockaddr* server_addr, socklen_t sin_size) {
    int bytes_sent = sendto(client_sock, message, strlen(message), 0, server_addr, sin_size);
    if (bytes_sent < 0) {
        perror("Error: ");
    }
    return bytes_sent;
}

// Function to receive a message from the server
int receiveMessage(int client_sock, char* buffer, const struct sockaddr* server_addr, socklen_t* sin_size) {
    int bytes_received = recvfrom(client_sock, buffer, BUFF_SIZE - 1, 0, (struct sockaddr *)server_addr, sin_size);
    if (bytes_received < 0) {
        perror("Error: ");
    } else {
        buffer[bytes_received] = '\0';
    }
    return bytes_received;
}

int main() {
    int client_sock;
    char buff[BUFF_SIZE];
    char username[BUFF_SIZE];
    struct sockaddr_in server_addr;
    socklen_t sin_size;
    int bytes_sent,bytes_received;
    // Create a UDP socket
    client_sock = createSocket();
    if (client_sock == -1) {
        return 1;
    }

    // Define the address of the server
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERV_IP);

    memset(buff, '\0', 30);
    strcpy(buff, "client1 connected\n"); // send init connection string

    sin_size = sizeof(struct sockaddr);

    // Send the init connection string to the server
    if (sendMessage(client_sock, buff, (struct sockaddr*)&server_addr, sin_size) < 0) {
        close(client_sock);
        return 0;
    }

    while(1){
        while (1){
            printf("Xin mời đăng nhập\n");
            printf("Insert username\n");
            memset(buff, '\0', 40);
            fgets(buff, BUFF_SIZE, stdin);
            strcpy(username, buff);
            //username[sizeof(username) - 1] = '\0';
            sin_size = sizeof(struct sockaddr);

            // Send the user input to the server
            if (sendMessage(client_sock, buff, (struct sockaddr*)&server_addr, sin_size) < 0) {
                close(client_sock);
                return 0;
            }
            memset(buff, 0, sizeof(buff)); // Hoặc: memset(buff, '\0', sizeof(buff));
            // Receive a response from the server
            int bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size); //receive message from server
            if(bytes_received < 0){
                    perror("Error 2");
                    return 0;
                }
            buff[bytes_received] = '\0';
            //buff[bytes_received - 1] = '\0';
            printf("%s\n", buff);
            if (strcmp(buff, "Account is blocked") == 0){
                printf("\nAccount not ready\n");
                continue;
            }
            while(strcmp(buff, "Sai tên đăng nhập") == 0){
                printf("Not ok, insert username\n");
                memset(buff, 0, sizeof(buff));
                fgets(buff, BUFF_SIZE, stdin);
                strcpy(username, buff);
                //username[sizeof(username) - 1] = '\0';
                if (sendMessage(client_sock, buff, (struct sockaddr*)&server_addr, sin_size) < 0) {
                    close(client_sock);
                    return 0;
                }
                bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size); //receive message from server
                if(bytes_received < 0){
                    perror("Error 2");
                    return 0;
                }
                buff[bytes_received] = '\0';
                
            }
            memset(buff, 0, sizeof(buff)); // Hoặc: memset(buff, '\0', sizeof(buff));

            //printf("Reply from server: %s", buff);
            printf("Insert password\n");
            memset(buff, '\0', 40);
            fgets(buff, BUFF_SIZE, stdin);

            // Send the user input to the server
            if (sendMessage(client_sock, buff, (struct sockaddr*)&server_addr, sin_size) < 0) {
                close(client_sock);
                return 0;
            }

            // Receive a response from the server
            if (recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size) < 0) {
                close(client_sock);
                return 0;
            }
            //buff[bytes_received] = '\0';
            //buff[bytes_received - 1] = '\0';
            printf("%s\n", buff);
            int pass_wrong_count = 0;
            while(strcmp(buff, "Sai pass") == 0){
                pass_wrong_count++;
                printf("pass_wrong_count: %d\n", pass_wrong_count);
                if(pass_wrong_count == 3){
                    break;
                }
                printf("Not ok, insert password\n");
                memset(buff, 0, sizeof(buff));
                fgets(buff, BUFF_SIZE, stdin);
                if (sendMessage(client_sock, buff, (struct sockaddr*)&server_addr, sin_size) < 0) {
                    close(client_sock);
                    return 0;
                }
                int bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size); //receive message from server
                if(bytes_received < 0){
                    perror("Error 2");
                    return 0;
                }
                buff[bytes_received] = '\0';
            }
            memset(buff, 0, sizeof(buff)); // Hoặc: memset(buff, '\0', sizeof(buff));
            if (pass_wrong_count == 3){
                printf("Account is blocked now\n\n");
                continue;
            }
            else {
                printf("OK, login successfully \n");
                break;
                if (sendMessage(client_sock, buff, (struct sockaddr*)&server_addr, sin_size) < 0) {
                    close(client_sock);
                    return 0;
                }
            }
            
        }
        printf("Bạn đã đăng nhập thành công\n");
        printf("Từ thời điểm này bạn có thể nhập mật khẩu mới\n");
        printf("Hoặc nhập 'bye' để thoát đăng nhập\n");
        while(1){
            printf("Nhập mật khẩu mới\n");
           memset(buff, '\0', BUFF_SIZE);
           fgets(buff, BUFF_SIZE, stdin);
           //buff[sizeof(buff) - 1] = '\0';
           // Loại bỏ dấu xuống dòng và khoảng trắng từ chuỗi
            for (int i = 0; buff[i]; i++) {
                if (buff[i] == '\n' || buff[i] == ' ') {
                    buff[i] = '\0';
                }
            }
           //printf("%s\n", buff);
           int checkout = strcmp(buff, "bye");
           if (checkout == 0){
                if(sendMessage(client_sock, buff, (struct sockaddr*)&server_addr, sin_size) < 0) {
                    close(client_sock);
                    return 0;
                }
                printf("Bye %s\n", username);
                printf("Sign out successfully\n");
                break;
            }
            if(checkValid(buff) == 0){
                printf("Error\n");
                continue;
            } else if (checkValid(buff) == 1) {
                if(sendMessage(client_sock, buff, (struct sockaddr*)&server_addr, sin_size) < 0) {
                    close(client_sock);
                    return 0;
                }
            }
            
            //printf("")
        }
    }
    
    // do {
    //     printf("Insert string to send:"); // input sent string
    //     memset(buff, '\0', (strlen(buff) + 1));
    //     fgets(buff, BUFF_SIZE, stdin);

    //     if (strcmp(buff, "@\n") == 0 || strcmp(buff, "#\n") == 0) { // exit if input string is @ or #
    //         break;
    //     }
    //     sin_size = sizeof(struct sockaddr);

    //     // Send the user input to the server
    //     if (sendMessage(client_sock, buff, (struct sockaddr*)&server_addr, sin_size) < 0) {
    //         close(client_sock);
    //         return 0;
    //     }

    //     // Receive a response from the server
    //     if (receiveMessage(client_sock, buff, (struct sockaddr*)&server_addr, &sin_size) < 0) {
    //         close(client_sock);
    //         return 0;
    //     }
    //     printf("Reply from server: %s", buff);
    // } while (strcmp(buff, "@") != 0 && strcmp(buff, "#") != 0);

    // Close the socket
    close(client_sock);
    return 0;
}
