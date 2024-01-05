#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h> 
//Thư viện cho phép sử dụng các hàm và cấu trúc để giải quyết tên miền.
#include <netinet/in.h>
//  Thư viện chứa các cấu trúc và hằng số liên quan đến giao thức Internet.
#include <arpa/inet.h>
// Thư viện chứa các hàm và cấu trúc để xử lý địa chỉ IP và số cổng.

int main(int argc, char *argv[]) {
    struct hostent *host_entry; 
    // struct hostent chứa thông tin về về một máy chủ, bao gồm offical name, alias name, kiểu địa chỉ và danh sách IP máy chủ
    // con trỏ host_entry sẽ chứa thông tin về máy chủ sau khi thực hiện các lệnh bên dưới
    struct in_addr **addr_list;
    // struct in_addr đại diện cho 1 địa chỉ IPv4. addr_list là một mảng con trỏ chứa danh sách địa chỉ IP mỗi máy chủ
    // dùng trong trường hợp tên miền -> IP
    struct in_addr ipv4addr;
    // dùng trong trường hợp IP -> tên miền

    // Kiểm tra số lượng tham số đầu vào
    if (argc != 2) {
        printf("Sai số lượng đối số truyền vào. Nhập lại theo cú pháp ./resolver B\n");
    } else {
        char xaunhapvao[100];
        // Sao chép tham số đầu vào từ dòng lệnh
        strcpy(xaunhapvao, argv[1]);

        // Kiểm tra xem tham số có phải là địa chỉ IP không
        if (inet_addr(xaunhapvao) == -1) {
            host_entry = gethostbyname(xaunhapvao);
            if (host_entry != NULL) {
                addr_list = (struct in_addr **)host_entry->h_addr_list;
                printf("Official IP: %s\n", inet_ntoa(*addr_list[0]));
                printf("Alias IP:\n");
                for (int i = 1; addr_list[i] != NULL; i++) {
                    printf("%s \n", inet_ntoa(*addr_list[i]));
                }
            } else {
                printf("Not found information\n");
            }
        } else {
            // Chuyển địa chỉ IP từ chuỗi sang cấu trúc in_addr
            inet_pton(AF_INET, xaunhapvao, &ipv4addr);
            host_entry = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
            if (host_entry != NULL) {
                printf("Official name: %s\n", host_entry->h_name);
                printf("Alias name:\n");
                while (*host_entry->h_aliases) {
                    printf("%s\n", *host_entry->h_aliases);
                    host_entry->h_aliases++;
                }
            } else {
                printf("Not found information\n");
            }
        }
    }

    return 0;
}
