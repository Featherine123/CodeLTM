

#include <stdio.h>
#include <stdlib.h>  
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
//#define PORT 5500
#define MAX_CLIENTS 5
#define MAX_LENGTH 80
#define MAX_USERNAME_LENGTH 80
#define MAX_PASSWORD_LENGTH 80

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int status;
    int wrongCount; 
    int isLogined; // 1: đã đăng nhập, 0: chưa đăng nhập
} Account;

typedef struct Node {
    Account data;
    struct Node* next;
} Node;

static Node* head = NULL; // danh sách liên kết


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
// hàm tìm kiếm tài khoản
Node* findAccount(char* username){
    Node* current = head;
    while (current != NULL){
       // printf("current node data: %s\n", current->data.username);
        if (strcmp(current->data.username, username) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Xử lý các yêu cầu login/logout từ client 
void handle_client(int sockfd){
	 Node *loginAccount = NULL;
	readFile();
  // code xử lý authen và login/logout
  // ...
	ssize_t  bytes_received, bytes_sent;
	char buff[MAX_LENGTH];
	while(1){
		bytes_received =  recv(sockfd, buff, MAX_LENGTH, 0);
		if (bytes_received <= 0){
			printf("bytes_received <0 \n");
			return;
		}
		printf("Tên đăng nhập là: %s\n", buff);

		loginAccount = findAccount(buff);
		printf("isLogined1: %d\n", loginAccount->data.isLogined);
		if (loginAccount == NULL){
			printf("Cannot find account\n");
			strcpy(buff, "Cannot find account");
			bytes_sent = send(sockfd, buff, sizeof(buff), 0);
			if (bytes_sent < 0){
				printf("Không gửi được thông báo cho client\n");
				return;
			}
			continue;
		}

		if (loginAccount->data.status == 0){
			printf("Account is blocked\n");
			strcpy(buff, "Account is blocked");
			bytes_sent = send(sockfd, buff, sizeof(buff), 0);
			if (bytes_sent < 0){
				printf("Không gửi được thông báo cho client\n");
				return;
			}
			continue;
		}

		if (loginAccount->data.isLogined == 1){
			printf("Account is logined\n");
			strcpy(buff, "Account is logined");
			bytes_sent = send(sockfd, buff, sizeof(buff), 0);
			if (bytes_sent < 0){
				printf("Không gửi được thông báo cho client\n");
				return;
			}
			continue;
		}
		strcpy(buff, "Enter your password");
		bytes_sent = send(sockfd, buff, sizeof(buff), 0);

		memset(buff, '\0', MAX_LENGTH);
		bytes_received =  recv(sockfd, buff, MAX_LENGTH, 0);
		if (bytes_received <= 0){
			printf("bytes_received <0 \n");
			return;
		}
		printf("mat khau la: %s\n", buff);

		int wrongTime ;
		int isCorrect = 1;
		if (strcmp(buff, loginAccount->data.password) != 0){
			isCorrect = 0;
			wrongTime = 1;
			while(wrongTime < 3 ){
				wrongTime++;
				strcpy(buff, "wrong");
				bytes_sent = send(sockfd, buff, sizeof(buff), 0);
				if (bytes_sent < 0){
					printf("Không gửi được thông báo cho client\n");
					return;
				}

				memset(buff, '\0', MAX_LENGTH);
				bytes_received =  recv(sockfd, buff, MAX_LENGTH, 0);
				if (bytes_received <= 0){
					printf("bytes_received <0 \n");
					return;
				}
				printf("mat khau la: %s\n", buff);
				if (strcmp(buff, loginAccount->data.password) == 0) {
					isCorrect = 1;
					// strcpy(buff, "success");
					// bytes_sent = send(sockfd, buff, sizeof(buff), 0);
					// if (bytes_sent < 0){
					// 	printf("Không gửi được thông báo cho client\n");
					// 	return;
					// }
					break;
				}
			}
			//printf("da thoat vong while check 3 lan\n");
		}
		// da thoat khoi if
		memset(buff, '\0', MAX_LENGTH);
		if (isCorrect == 0){
			printf("blocked\n");
			strcpy(buff, "blocked");
			bytes_sent = send(sockfd, buff, sizeof(buff), 0);
			loginAccount->data.status = 0;
			writeAccountsToFile();
			continue;
		}
		else {
			printf("login successfully\n");
			loginAccount->data.isLogined = 1;
			printf("isLogined2: %d\n", loginAccount->data.isLogined);
			strcpy(buff, "success");
			bytes_sent = send(sockfd, buff, sizeof(buff), 0);
			if (bytes_sent < 0){
				printf("Không gửi được thông báo cho client\n");
				return;
			}
			bytes_received =  recv(sockfd, buff, MAX_LENGTH, 0);
			if (bytes_received <= 0){
				printf("Client đã đột ngột tắt \n");
				loginAccount->data.isLogined = 0;
				return;
			}
			if (strcmp(buff, "logout") == 0){
				printf("loggout\n");
				loginAccount->data.isLogined = 0;
				memset(buff, '\0', MAX_LENGTH);
				strcpy(buff, "goodbye");
				bytes_sent = send(sockfd, buff, sizeof(buff), 0);
				if (bytes_sent < 0){
					printf("Không gửi được thông báo cho client\n");
					return;
				}
			}

		}
		

	}
	
}

// Tạo socket TCP và lắng nghe
void init_server(int port_number){
  int server_fd, new_socket; 
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  // Tạo socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }
    
  // Cấu hình socket
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  //address.sin_addr.s_addr = INADDR_ANY;
  in_addr_t inet_addr(const char *cp);
  address.sin_addr.s_addr = inet_addr("127.0.0.1");
  address.sin_port = htons( port_number );
    
  // Gán socket với IP và cổng
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Bắt đầu lắng nghe 
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  // Chấp nhận kết nối từ client
  while(1){
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    // Tạo tiến trình con để xử lý client
    if(!fork()) {
      close(server_fd);  
      handle_client(new_socket);
      exit(0);
    }
    else
      close(new_socket); 
  }
}



int main(int argc, char const *argv[]) {
  if (argc != 2){
	printf("Cần nhập đúng định dạng: ./server PortNumber\n");
	return 1;
  }
  char* port = argv[1];
  int port_number = atoi(port);
  init_server(port_number);
  return 0;
}


















