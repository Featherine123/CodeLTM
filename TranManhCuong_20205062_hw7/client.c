
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define PORT 5500
#define MAX_LENGTH 80
int main(int argc, char const *argv[])
{
  if (argc != 3){
    printf("Cần nhập đúng định dạng: ./client IPAddress PortNumber\n");
    return 1;
  }
  char* ip = argv[1];
  char* port = argv[2];
  int port_number = atoi(port);

  int sock;
  struct sockaddr_in server;

  // Tạo socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    printf("Could not create socket");
  }
  puts("Socket created");

  server.sin_addr.s_addr = inet_addr(ip);
  server.sin_family = AF_INET;
  server.sin_port = htons(port_number);

  // Kết nối tới server
  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("connect failed.");
    return 1;
  }

  // Gửi nhận dữ liệu với server
  // ....
  ssize_t  bytes_received, bytes_sent;
  char buff[MAX_LENGTH];
  char username[MAX_LENGTH];
  while(1){
    printf("Nhập tên đăng nhập\n");
    fgets(buff, MAX_LENGTH, stdin);
    int newline_pos = strcspn(buff, "\n");
    buff[newline_pos] = '\0'; 
    strcpy(username, buff);
    bytes_sent = send(sock, buff, sizeof(buff), 0);
    if (bytes_sent < 0){
      printf("Không gửi được thông báo cho server\n");
    }

    memset(buff, '\0', MAX_LENGTH);
    bytes_received =  recv(sock, buff, MAX_LENGTH, 0);
    if (bytes_received <= 0){
      printf("bytes_received <0 \n");
      return 1;
    }

    if (strcmp(buff, "Cannot find account") == 0){
      printf("Không tồn tại tài khoản này\n");
      continue;
    }
    if (strcmp(buff, "Account is blocked") == 0){
      printf("Tài khoản đã bị khoá\n");
      continue;
    }

    if (strcmp(buff, "Account is logined") == 0){
      printf("Tài khoản này đã đăng nhập\n");
      continue;
    }
    if (strcmp(buff, "Enter your password") == 0){
      memset(buff, '\0', MAX_LENGTH);
      printf("Xin mời nhập mật khẩu\n");
      fgets(buff, MAX_LENGTH, stdin);
      int newline_pos = strcspn(buff, "\n");
      buff[newline_pos] = '\0'; 
      bytes_sent = send(sock, buff, sizeof(buff), 0);
      if (bytes_sent < 0){
        printf("Không gửi được thông báo cho server\n");
      }


      memset(buff, '\0', MAX_LENGTH);
      bytes_received =  recv(sock, buff, MAX_LENGTH, 0);
      if (bytes_received <= 0){
        printf("bytes_received <0 \n");
        return 1;
      }
      while (strcmp(buff, "wrong") == 0){
        memset(buff, '\0', MAX_LENGTH);
        printf("Xin mời nhập mật khẩu\n");
        fgets(buff, MAX_LENGTH, stdin);
        int newline_pos = strcspn(buff, "\n");
        buff[newline_pos] = '\0'; 
        bytes_sent = send(sock, buff, sizeof(buff), 0);
        if (bytes_sent < 0){
          printf("Không gửi được thông báo cho server\n");
        }
        memset(buff, '\0', MAX_LENGTH);
        bytes_received =  recv(sock, buff, MAX_LENGTH, 0);
        if (bytes_received <= 0){
          printf("bytes_received <0 \n");
          return 1;
        }
        //printf("thong bao cuar server %s\n", buff);
      }

      if (strcmp(buff, "blocked") == 0){
        printf("Tài khoản của bạn đã bị khoá \n");
        continue;
      }
      int choice;
      if (strcmp(buff, "success") == 0){
        printf("Bạn đã đăng nhập thành công\n");
        printf("Bạn có thể gõ 1 để lựa chọn đăng xuất tài khoản\n");
        printf("Nếu lựa chọn khác sẽ quay lại trang đăng nhập\n");
        printf("Do mỗi client chỉ có thể đăng nhập 1 tài khoản tại 1 thời điểm nên hiện tại có đúng 1 option \n");
        scanf("%d", &choice);
        // fgets(buff, MAX_LENGTH, stdin);
        // int newline_pos = strcspn(buff, "\n");
        // buff[newline_pos] = '\0'; 

        memset(buff, '\0', MAX_LENGTH);
        if (choice == 1){
          strcpy(buff, "logout");
          bytes_sent = send(sock, buff, sizeof(buff), 0);
          memset(buff, '\0', MAX_LENGTH);
          bytes_received = recv(sock, buff, MAX_LENGTH, 0);
          if (bytes_received <= 0){
            printf("bytes_received <0 \n");
            return 1;
          }
          if (strcmp(buff, "goodbye") == 0){
            printf("goodbye %s\n\n", username);
          }
          memset(buff, '\0', MAX_LENGTH);
        } 
      }

    }
  }
  
  close(sock);
  return 0;
}













