// /*UDP Echo Server*/
#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#define PORT 5550  /* Port that will be opened */ 
#define BUFF_SIZE 1024
#define SHA256_DIGEST_LENGTH 32
#include <stdlib.h>
#include <string.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int status;
    int wrongCount; // biến đếm số lần người dùng nhập sai pass trên mỗi tài khoản
    // làm như vậy thì có thể xét được trh ng dùng đăng nhập account1 sai, account 2 sai, account1 sai 
    //-> số lần sai của account 1 là 2 không phải 3
    int isLogined; // 1: đã đăng nhập, 0: chưa đăng nhập
} Account;



typedef struct Node {
    Account data;
    struct Node* next;
} Node;

Node* head = NULL; // danh sách liên kết

// hàm đọc file lấy dữ liệu khi khởi tạo chương trình
void readFile(){
    FILE *file;
    file = fopen("account.txt","r+");
    if (file == NULL) {
        printf("Cannot open file account.txt\n");
        //exit(1);
        return;
    }
     while (!feof(file)) {
        Account acc;
        if (fscanf(file, "%s %s %d", acc.username, acc.password, &acc.status) != 3) {
            break;
        }
        //printf("%s %s %d\n", acc.username, acc.password, acc.status);
        acc.wrongCount = 0;
        acc.isLogined = 0; // chưa đăng nhập
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = acc;

        if(head == NULL) head = newNode;
        else {
            Node* temp = head;
            while(temp->next != NULL) temp = temp->next;
            newNode->next = NULL;
            temp->next = newNode;
        }
        

    }
    
    fclose(file);
}

// hàm tìm kiếm tài khoản
Node* findAccount(char* username){
	head = NULL;
	readFile();
    Node* current = head;
   // printf("Xau duyet la: %s\n", username);
    while (current != NULL){
        //printf("current node data: %s\n", current->data.username);
        if (strcmp(current->data.username, username) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// viết lại file để update khi người dùng đăng ký
void writeAccountsToFile(){
    FILE *file = fopen("account.txt", "w");
    if(file == NULL){
        printf("Cannot open file account.txt\n");
        return;
    }

    Node* current = head;
    while (current != NULL) {
        fprintf(file, "%s %s %d\n", current->data.username, current->data.password, current->data.status);
        current = current->next;
    }
    

    fclose(file);
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

void SHA256ToHex(const unsigned char* sha256Hash, char* sha256Hex) {
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(&sha256Hex[i * 2], 3, "%02x", sha256Hash[i]);
    }
}

void SHA256_1(const char* data, unsigned char* sha256Hash) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;

    OpenSSL_add_all_digests(); // Khởi tạo thư viện OpenSSL cho các thuật toán băm

    md = EVP_get_digestbyname("sha256");
    if (!md) {
        printf("Unknown message digest\n");
        exit(1);
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, data, strlen(data));
    EVP_DigestFinal_ex(mdctx, sha256Hash, NULL);
    EVP_MD_CTX_free(mdctx);
}
void extract_numbers_and_characters(const char* md5Hex, char* numbers, char* characters) {
    int k = 0;
    int z = 0;
    for (int i = 0; i < strlen(md5Hex); i++) {
        if (md5Hex[i] >= '0' && md5Hex[i] <= '9') {
            numbers[k] = md5Hex[i];
            k++;
        } else if ((md5Hex[i] >= 'a' && md5Hex[i] <= 'z') || (md5Hex[i] >= 'A' && md5Hex[i] <= 'Z')) {
            characters[z] = md5Hex[i];
            z++;
        }
    }
    numbers[k+1] = '\0';
    characters[z+1]='\0';
}
int main() {
	int sendBytes;
	int rcvBytes;
	int server_sock; /* file descriptors */
	char buff[BUFF_SIZE];
	char done[BUFF_SIZE];
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client2; /* client's address information */
	struct sockaddr_in client1;
	socklen_t sin_size;
	Node* loginAccount;
	int wrongCount = 0;
	//int checkTK = 1;
	printf("Chú ý chạy server trước client không thì sẽ không thực hiện được chương trình\n");
	//Step 1: Construct a UDP socket
	if ((server_sock=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 1;
	}
	
	//Step 2: Bind address to socket
	server.sin_family = AF_INET;         
	server.sin_port = htons(PORT);   /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = INADDR_ANY;  /* INADDR_ANY puts your IP address automatically */   
	bzero(&(server.sin_zero),8); /* zero the rest of the structure */

  
	if(bind(server_sock,(struct sockaddr*)&server,sizeof(struct sockaddr))==-1){ /* calls bind() */
		perror("\nError: ");
		return 1;
	}     
	
	 sin_size = sizeof(struct sockaddr_in);
    rcvBytes = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client1, &sin_size);
	if (rcvBytes < 0)
		perror("\nError: ");
	else{
		buff[rcvBytes] = '\0';
		printf("[%s:%d]: %s", inet_ntoa(client1.sin_addr), ntohs(client1.sin_port), buff);
	}
	rcvBytes = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client2, &sin_size);	
	if (rcvBytes < 0)
		perror("\nError: ");
	else{
		buff[rcvBytes] = '\0';
		printf("[%s:%d]: %s", inet_ntoa(client2.sin_addr), ntohs(client2.sin_port), buff);
	}
	while(1){
		// check tài khoản
		while (1){
		char status[100] = "";
		rcvBytes = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client1, &sin_size);
		memset(done, '\0', sizeof(done));
		if (rcvBytes < 0)
			perror("\nError: ");
		else{
			buff[rcvBytes - 1] = '\0';
			//buff[rcvBytes] = '\0';
			printf("[%s:%d]: %s\n", inet_ntoa(client1.sin_addr), ntohs(client1.sin_port), buff);
			loginAccount = findAccount(buff);
			if (loginAccount == NULL) {
				strcpy(status, "Sai tên đăng nhập");
				sendBytes = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client1, sin_size);
				printf("\nAccount is not exist\n");
				memset(status, '\0', sizeof(status));
				continue;
			} else {
				if (loginAccount->data.status == 0){
					printf("Account is blocked\n");
					strcpy(status, "Account is blocked");
					sendBytes = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client1, sin_size);
					memset(status, '\0', sizeof(status));
					continue;
				}
				else {
					strcpy(status, "ok");
					sendBytes = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client1, sin_size);
					if (sendBytes < 0){
						perror("Error: ");
						close(server_sock);
						return 0;
					}
					//checkTK = 0;
					break;
				}
				
			}
		}
		}
		int isLogined = 0;
		// check mật khẩu
		while (1){
		char status[100] = "";
		rcvBytes = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client1, &sin_size);
		memset(done, '\0', sizeof(done));
		if (rcvBytes < 0)
			perror("\nError: ");
		else{
			buff[rcvBytes - 1] = '\0';
			printf("[%s:%d]: %s\n", inet_ntoa(client1.sin_addr), ntohs(client1.sin_port), buff);
			if (strcmp(buff, loginAccount->data.password) == 0 ) {
				strcpy(status, "ok");
				sendBytes = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client1, sin_size);
				if (sendBytes < 0)
				{
					perror("Error: ");
					close(server_sock);
					return 0;
				}
				wrongCount = 0;
				isLogined = 1;
				break;
			} else {
				wrongCount++;
				isLogined = 0;
				if (wrongCount == 3){
					loginAccount->data.status = 0;
					writeAccountsToFile();
					strcpy(status, "Sai pass");
					sendBytes = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client1, sin_size);
					break;
				}
				printf("\nSai mật khẩu\n");
				strcpy(status, "Sai pass");
				sendBytes = sendto(server_sock, status, strlen(status), 0,(struct sockaddr *) &client1, sin_size);
			}
		}
		}
		// nếu đăng nhập thành công thì cho người dùng đổi mật khẩu
		if(isLogined == 1) printf("Đã đăng nhập thành công\n");
		else printf("Account is blocked\n");
		while (isLogined)
		{
			memset(buff, '\0', sizeof(buff));
			rcvBytes = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client1, &sin_size);
			//buff[rcvBytes - 1] = '\0';
			for (int i = 0; buff[i]; i++) {
                if (buff[i] == '\n' || buff[i] == ' ') {
                    buff[i] = '\0';
                }
            }
			if (strcmp(buff, "bye") == 0){
				printf("Goodbye %s\n", loginAccount->data.username);
				isLogined = 0;
				break;
			} else {

				strcpy(loginAccount->data.password, buff);
				writeAccountsToFile();
				unsigned char sha256Hash[SHA256_DIGEST_LENGTH];
				char sha256Hex[2 * SHA256_DIGEST_LENGTH + 1]; 

    			SHA256_1(buff, sha256Hash);

    			SHA256ToHex(sha256Hash, sha256Hex);	// exchange hash to hex

				char numbers[100] = "";  // Initialize with empty string
				char characters[100] = "";  // Initialize with empty string
				extract_numbers_and_characters(sha256Hex, numbers, characters); // extract characters and numbers from
				if(strlen(numbers) > 0){
					sendBytes = sendto(server_sock, numbers, strlen(numbers), 0,(struct sockaddr *) &client2, sin_size);
					if (sendBytes < 0) {
						perror("Error: ");
						close(server_sock);
						return 0;
					}
				}
				if(strlen(characters) > 0) {
					sendBytes = sendto(server_sock, characters, strlen(characters), 0,(struct sockaddr *) &client2, sin_size);
					if (sendBytes < 0) {
						perror("Error: ");
						close(server_sock);
						return 0;
					}
				}
			}
		}
		
	
	}
	
		
}
















// while(1){ // infininite receive syn from clients
	// 	rcvBytes = recvfrom(server_sock, buff, BUFF_SIZE-1, 0, (struct sockaddr *) &client1, &sin_size);
	// 	if (rcvBytes < 0)
	// 		perror("\nError: ");
	// 	else{
	// 		buff[rcvBytes] = '\0';
	// 		printf("[%s:%d]: %s", inet_ntoa(client1.sin_addr), ntohs(client1.sin_port), buff);
	// 	}
	// 	memset(done, '\0', sizeof(done));

	// 	if (rcvBytes < 0)
	// 		perror("\nError: ");
	// 	else{
	// 		buff[rcvBytes] = '\0';
	// 		printf("[%s:%d]: %s", inet_ntoa(client1.sin_addr), ntohs(client1.sin_port), buff);
	// 	}
		
	// 	if(checkValid(buff) == 0){
	// 		strcpy(done,"error\n");
	// 		sendBytes = sendto(server_sock, done, strlen(done), 0,(struct sockaddr *) &client1, sin_size); //error message
			
	// 		if(sendBytes < 0){
	// 			perror("Error: ");
	// 			close(server_sock);
	// 			return 0;
	// 		}
	// 	} else {
	// 		strcpy(done,"ok\n");
	// 		sendBytes = sendto(server_sock, done, strlen(done), 0,(struct sockaddr *) &client1, sin_size); //error message
	// 		if(sendBytes < 0){
	// 			perror("Error: ");
	// 			close(server_sock);
	// 			return 0;
	// 		}
	// 	unsigned char md5Hash[MD5_DIGEST_LENGTH];  // encode

    //     char md5Hex[2 * MD5_DIGEST_LENGTH + 1];

    //     MD5_1(buff, md5Hash);

    //     MD5ToHex(md5Hash, md5Hex);	// exchange hash to hex

	// 	char numbers[100] = "";  // Initialize with empty string
    // 	char characters[100] = "";  // Initialize with empty string
    	
	// 	extract_numbers_and_characters(md5Hex, numbers, characters); // extract characters and numbers from
	// 		if(strlen(numbers) > 0){
	// 			sendBytes = sendto(server_sock, numbers, strlen(numbers), 0,(struct sockaddr *) &client2, sin_size);
	// 			if (sendBytes < 0)
	// 			{
	// 				perror("Error: ");
	// 				close(server_sock);
	// 				return 0;
	// 			}
	// 		}
	// 		if(strlen(characters) > 0){
	// 			sendBytes = sendto(server_sock, characters, strlen(characters), 0,(struct sockaddr *) &client2, sin_size);
	// 			if (sendBytes < 0)
	// 			{
	// 				perror("Error: ");
	// 				close(server_sock);
	// 				return 0;
	// 			}
	// 		}
	// 	}
    // }
