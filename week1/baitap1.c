#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int status;
    int wrongCount;
} Account;



typedef struct Node {
    Account data;
    struct Node* next;
} Node;

Node* head = NULL; // danh sách liên kết
Node* backupHead = NULL;
int isLoggined = 0; // 0: chưa đăng nhập, 1: đăng nhập rồi

int firstTimeReadFile = 0;

void restoreWrongCount(){
    Node* current = head;
    while(current != NULL){
        if (backupHead != NULL) {
            printf("backupHead: %s, current: %s\n", backupHead->data.username, current->data.username);
        } else printf("current: %s\n", current->data.username);
        if(current == backupHead) printf("backup thanh cong\n");
        current = current->next;
        backupHead = backupHead->next;
    }
    

}
void readFile(){
    if (firstTimeReadFile != 0) backupHead = head;
    head = NULL;
    FILE *file;
    file = fopen("account.txt","r+");
    if (file == NULL) {
        printf("Khong the mo file account.txt\n");
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
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = acc;
        newNode->next = head;
        head = newNode;

    }
    if (firstTimeReadFile == 0) backupHead = head;
    else restoreWrongCount();
    firstTimeReadFile++;
    fclose(file);
}

int checkHasAccount(char* username){
    readFile();
    Node* current = head;
    while(current != NULL){
        if(strcmp(current->data.username, username) == 0){
            return 1;
        }
        current = current->next;
    }
    return 0;
}
void writeAccountsToFile(){
    FILE *file = fopen("account.txt", "w");
    if(file == NULL){
        printf("khong the mo file account.txt\n");
        return;
    }

    Node* current = head;
    while (current != NULL) {
        fprintf(file, "%s %s %d\n", current->data.username, current->data.password, current->data.status);
        current = current->next;
    }

    fclose(file);
}
void userRegister() {
    printf("Register function\n");
    Account newAccount;
    printf("Nhập tên tài khoản \n");
    scanf("%s", newAccount.username);
    

    if(checkHasAccount(newAccount.username) == 1){
        printf("Tài khoản đã tồn tại\n");
        return;
    }
    else {
        printf("Nhập mật khẩu\n");
        scanf("%s", newAccount.password);
        newAccount.status = 1;
        newAccount.wrongCount = 0;

        Node* temp = head;
        while(temp->next != NULL) temp = temp->next;
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = newAccount;
        temp->next = newNode;
        //newNode->next = head;
        //head = newNode;

        writeAccountsToFile();
        printf("Dang ky tai khoan thanh cong!\n");
    }
    
}
Node* findAccount(char* username){
    readFile(); // cập nhật lại giá trị db phòng trh người dùng mới tạo acc mới
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
Node* findAccountWrongPass(char* username){

}
void signin() {
    printf("Sign in function\n");
    char logginUsername[MAX_USERNAME_LENGTH];
    char logginPassword[MAX_PASSWORD_LENGTH];
    printf("Xin mời nhập tài khoản đăng nhập\n");
    scanf("%s", logginUsername);

    Node* logginAccount = findAccount(logginUsername);
    if(logginAccount == NULL){
        printf("Tài khoản không tồn tại, xin hãy đăng ký trước đã\n");
        return;
    }
    printf("Xin mời nhập mật khẩu\n");
    scanf("%s", logginPassword);
    if (strcmp(logginAccount->data.password, logginPassword) == 0){
        printf("Đăng nhập thành công\n");
        logginAccount->data.wrongCount = 0;
        return;
    } else {
        printf("Sai mật khẩu, ");
        //logginAccount->data.wrongCount = logginAccount->data.wrongCount + 1;

        if(logginAccount->data.wrongCount == 3) {
            printf("tài khoản của bạn đã bị khoá \n");
            logginAccount->data.status = 0;
            writeAccountsToFile();
        } else {
            printf("bạn còn %d lần thử\n", (3 - logginAccount->data.wrongCount));
        }
    }

}


void search() {
    printf("Search\n");
    char username[MAX_USERNAME_LENGTH];

    printf("Nhap ten tai khoan (username): ");
    scanf("%s", username);

    Node* account = findAccount(username);
    if (account == NULL) {
        printf("Tai khoan khong ton tai!\n");
    } else {
        printf("Trang thai tai khoan: %s\n", account->data.status == 1 ? "active" : "blocked");
    }

}

void signout() {
    printf("Sign out\n");
}

int main() {
    int selected = 0;
    do {
        printf("USER MANAGEMENT PROGRAM\n");
        printf("-----------------------------------\n");
        printf("1. Register\n");
        printf("2. Sign in\n");
        printf("3. Search\n");
        printf("4. Sign out\n");

        printf("Input your choice: ");
        scanf("%d", &selected);

        switch (selected) {
            case 1:
                userRegister();
                break;
            case 2:
                signin();
                break;
            case 3:
                search();
                break;  
            case 4:
                signout();
                break;  
            default:
                break;
        }
    } while (selected >= 1 && selected <= 4);

    return 0;
}













