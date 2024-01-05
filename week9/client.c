#include <stdio.h>
#include <stdlib.h>  
#include <string.h>
// #include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include <openssl/aes.h>


#define MAX_MESSAGE_SIZE 1024
#define AES_KEY_SIZE 128
#define AES_BLOCK_SIZE 16
// write a functio to print hello world
void printHelloWorld(){
    printf("Hello world\n");
}
// AES key for encryption and decryption
const static unsigned char aes_key[] = "laptrinhmangweek9";

void encryptAES(const unsigned char *plaintext, unsigned char *ciphertext) {
    AES_KEY aesKey;
    AES_set_encrypt_key(aes_key, AES_KEY_SIZE, &aesKey);
    AES_encrypt(plaintext, ciphertext, &aesKey);
}
void decryptAES(const unsigned char *ciphertext, unsigned char *plaintext) {
    AES_KEY aesKey;
    AES_set_decrypt_key(aes_key, AES_KEY_SIZE, &aesKey);
    AES_decrypt(ciphertext, plaintext, &aesKey);
}

void *receiveData(void *arg) {
    int clientSocket = *((int *)arg);
    char receivedMessage[MAX_MESSAGE_SIZE];
    char encrypted_text[MAX_MESSAGE_SIZE];
   // unsigned char decrypted_text[AES_BLOCK_SIZE];
    while (1) {
        //ssize_t bytesRead = recv(clientSocket, receivedMessage, sizeof(receivedMessage) - 1, 0);
        ssize_t bytesRead = recv(clientSocket, encrypted_text, AES_BLOCK_SIZE, 0);
        if (bytesRead <= 0) {
            printf("Error receiving data");
            break;
        }
        //receivedMessage[bytesRead] = '\0';
        // Decrypt the string
    	//decryptAES(receivedMessage, decrypted_text);
    	// giải mã xâu aes
    	unsigned char decrypted_text[AES_BLOCK_SIZE];
		decryptAES(encrypted_text, decrypted_text);
		printf("encrypted_text: %s\n", encrypted_text);
        printf("decrypted_text: %s\n", decrypted_text);
        
    }

    return NULL;
}


int main(int argc, char const *argv[]){
    if (argc != 3){
    	printf("Cần nhập đúng định dạng: ./client IPAddress PortNumber\n");
    	return 1;
    }
    

    char* ip = argv[1];
    char* port = argv[2];
    int port_number = atoi(port);

    // Tạo socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        printf("Error creating socket");
        return 0;
    }

    // Thiết lập địa chỉ của server
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &(serverAddr.sin_addr));

    // Kết nối đến server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        printf("Error connecting to server");
        return 0;
    }

    printf("Connected to the server.\n");

     // Tạo luồng cho việc nhận dữ liệu từ server
    pthread_t receiveThread;
    pthread_create(&receiveThread, NULL, receiveData, (void *)&clientSocket);

    // Bắt đầu gửi và nhận dữ liệu với server
    char message[MAX_MESSAGE_SIZE];
    
    while (1) {
        // Nhập tin nhắn từ người dùng
        unsigned char encrypted_text[AES_BLOCK_SIZE];
        
        fgets(message, sizeof(message), stdin);
        
        // Xóa ký tự xuống dòng
		int newline_pos = strcspn(message, "\n");  
		message[newline_pos] = '\0';

		// Xóa khoảng trắng thừa ở cuối
		int space_pos = strlen(message) - 1;
		while(message[space_pos] == ' ' && space_pos > 0) {
  			message[space_pos--] = '\0'; 
		}
   		// mã hoá xâu message aes
        // Gửi tin nhắn tới server
        encryptAES((const unsigned char *)message, encrypted_text);
        send(clientSocket, encrypted_text, AES_BLOCK_SIZE, 0);
        //send(clientSocket, message, strlen(message), 0);
        //printf("Enter message (or 'exit' to quit): ");
        
    }
    // Đóng kết nối
    //close(clientSocket);

    // Chờ kết thúc luồng nhận dữ liệu
    pthread_join(receiveThread, NULL);

    return 0;
}



































