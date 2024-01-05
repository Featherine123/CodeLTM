#include <stdio.h>
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

// hàm đăng ký tài khoản
void userRegister() {
    Account newAccount;
    printf("Enter your username \n");
    scanf("%s", newAccount.username);
    
    Node* resAccount = findAccount(newAccount.username);
    if(resAccount != NULL){
        printf("Account existed, please create other account\n");
        return;
    }
    else {
        printf("Enter your password\n");
        scanf("%s", newAccount.password);
        newAccount.status = 1;
        newAccount.wrongCount = 0;

        Node* temp = head;
        while(temp->next != NULL) temp = temp->next;
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = newAccount;
        newNode->next = NULL;
        temp->next = newNode;
        
        writeAccountsToFile();
        printf("Successful registration!\n");
    }
    
}

// hàm đăng nhập
void signin() {
    char loginUsername[MAX_USERNAME_LENGTH];
    char loginPassword[MAX_PASSWORD_LENGTH];
    printf("Enter your username\n");
    scanf("%s", loginUsername);

    Node* loginAccount = findAccount(loginUsername);
    if(loginAccount == NULL){
        printf("Cannot find account\n");
        return;
    }
    if(loginAccount->data.status == 0){
        printf("Account is blocked\n");
        return;
    }
    printf("Enter your password\n");
    scanf("%s", loginPassword);
    if (strcmp(loginAccount->data.password, loginPassword) == 0){
        printf("Hello %s\n", loginUsername);
        loginAccount->data.wrongCount = 0;
        loginAccount->data.isLogined = 1;
        return;
    } else {
        printf("Password is incorrect, ");
        loginAccount->data.wrongCount = loginAccount->data.wrongCount + 1;

        if(loginAccount->data.wrongCount == 3) {
            printf("account is blocked \n");
            loginAccount->data.status = 0;
            writeAccountsToFile();
        } else {
            if (3 - loginAccount->data.wrongCount > 1)
            printf("you have %d times to try\n", (3 - loginAccount->data.wrongCount));
            else printf("you have %d time to try\n", (3 - loginAccount->data.wrongCount));
        }
    }

}

// hàm tìm kiếm
void search() {
    char username[MAX_USERNAME_LENGTH];

    printf("Enter account's username you want to search \n");
    scanf("%s", username);

    Node* account = findAccount(username);
    if (account == NULL) {
        printf("Cannot find account\n");
    } else {
        printf("Account is %s\n", account->data.status == 1 ? "active" : "blocked");
    }


}

// hàm đăng xuất
void signout() {
    //printf("Sign out\n");
    char signoutUsername[MAX_USERNAME_LENGTH] = "";
    printf("Enter username you want to signout\n");
    scanf("%s", signoutUsername);
    Node* signoutAccount = findAccount(signoutUsername);
    if(signoutAccount == NULL){
        printf("Account is not signin\n");
        return;
    } else {
        if(signoutAccount->data.isLogined == 1){
            printf("Goodbye %s\n", signoutUsername);
            signoutAccount->data.isLogined = 0;
        } else {
            printf("Account is not signin\n");
        }
    }

}
void freeMemory(){
    Node* current = head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp); // Giải phóng bộ nhớ của nút hiện tại
    }
    head = NULL; // Đặt con trỏ đầu danh sách về NULL để đảm bảo danh sách trống sau khi giải phóng.
}
int main() {
    readFile(); 
    int selected = 0; // lựa chọn của người dùng
    // menu
    do {
        printf("USER MANAGEMENT PROGRAM\n");
        printf("-----------------------------------\n");
        printf("1. Register\n");
        printf("2. Sign in\n");
        printf("3. Search\n");
        printf("4. Sign out\n");
        printf("5. Close the program\n");
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
            case 5:
                freeMemory();
                break;
            default:
                printf("Input other selection\n");
                break;
        }
    } while (selected != 5);

    // Tại đây, bạn có thể thêm mã để giải phóng bộ nhớ hoặc thực hiện các tác vụ khác trước khi kết thúc chương trình.

    return 0;
}
