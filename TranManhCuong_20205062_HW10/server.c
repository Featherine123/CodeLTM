#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <semaphore.h>
#include <pthread.h>

#define MAX_LINE_LENGTH 1024
#define SHA256_DIGEST_LENGTH 32
#define BACKLOG 20
#define BUFF_SIZE 1024


sem_t semaphore;
int opt = 1;
int conn_sock;
pthread_t tid;
int nready, client[FD_SETSIZE];
ssize_t ret;
fd_set readfds, allset;
char sendBuff[BUFF_SIZE], readBuff[BUFF_SIZE];
int i, listen_sock, maxi, maxfd, sockfd;
FILE *file;
char recv_data[BUFF_SIZE];
ssize_t bytes_sent, bytes_received;
struct sockaddr_in server;
struct sockaddr_in clientsock;
char buff[BUFF_SIZE];
char done[BUFF_SIZE];
socklen_t sin_size;
sem_t semaphore;

struct Account
{
    char username[1024];
    char password[1024];
    int status;
    int wrongPasswordCount;
    unsigned long isLogin;
    struct Account *next;
};

struct Account *head = NULL;
struct Account *currentLogin = NULL;

void *echo(void *arg);
void loadAccounts();
int searchAccount(const char *username);
int checkValid(const char *input);
void saveAccounts();
void signOut(unsigned long pid);
void signIn(int conn_sock, int index);
void signal_handler(int signal_num);

int sendData(int s, char *buff, int size, int flags);
int receiveData(int s, char *buff, int size, int flags);
void closeClientSocket(int index) {
    close(client[index]);
    FD_CLR(client[index], &allset);
    client[index] = -1; // Mark the socket as closed
    signOut(index);
}
int main(int argc, char *argv[])
{
    sem_init(&semaphore, 0, 1);
    if (argc != 2)
    {
        printf("Usage: ./server PortNumber\n");
        return 1;
    }

    char *port = argv[1];
    int port_number = atoi(port);

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("\nError creating socket");
        return 1;
    }

    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        close(listen_sock);
        return 1;
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port_number);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) != 0)
    {
        perror("Error binding");
        return 1;
    }

    if (listen(listen_sock, BACKLOG) == -1)
    {
        perror("Error listening");
        return 1;
    }

    if (signal(SIGTERM, signal_handler) == SIG_ERR)
    {
        perror("Could not register signal handler");
        return EXIT_FAILURE;
    }

    maxfd = listen_sock;
    maxi = -1;

    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listen_sock, &allset);

    while (1)
{
    readfds = allset;
    nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);

    if (nready < 0)
    {
        perror("select");
        return 1;
    }

    if (FD_ISSET(listen_sock, &readfds))
    {
        sin_size = sizeof(struct sockaddr_in);
        conn_sock = accept(listen_sock, (struct sockaddr *)&clientsock, &sin_size);

        if (conn_sock == -1)
        {
            perror("accept");
            continue;
        }
        
        // Print the client's IP address
        printf("You got a connection from %s\n", inet_ntoa(clientsock.sin_addr));

        for (i = 0; i < FD_SETSIZE; i++)
        {
            if (client[i] < 0)
            {
                client[i] = conn_sock;
                printf("%d\n", client[i]);
                break;
            }
        }

        if (i == FD_SETSIZE)
        {
            fprintf(stderr, "Too many clients\n");
            close(conn_sock);
        }

        FD_SET(conn_sock, &allset);

        if (conn_sock > maxfd)
        {
            maxfd = conn_sock;
        }

        if (i > maxi)
        {
            maxi = i;
        }

        // if (--nready <= 0)
        // {
        //     continue;
        // }
    }

    for (i = 0; i <= maxi; i++)
    {
        if ((sockfd = client[i]) < 0)
        {
            continue;
        }

        if (FD_ISSET(sockfd, &allset))
        {
            printf("%d\n", client[i]);
            signIn(sockfd, i);
            if (--nready <= 0)
            {
                break;
            }
            // Pass the correct index to the signIn function
        }
    }
}
    sem_destroy(&semaphore);
    return 0;
}

void signIn(int conn_sock,int index) {
    char username[1024];
    char password[1024];
    printf("1");
    while(1) {
        if (currentLogin != NULL && currentLogin->isLogin == index) {
            strcpy(done,"Ban co muon exits (y/n)\n");
            bytes_sent = send(conn_sock, done, strlen(done), 0);
            bytes_received = recv(conn_sock, buff, sizeof(recv_data), 0);
            if (bytes_received <= 0) {

            // Đóng kết nối và thực hiện các công việc cần thiết
                client[index] = -1;
               
                closeClientSocket(index);
                signOut(index);
                return ;
            }
            buff[bytes_received] = '\0';
            if (strcmp(buff,"y") == 0) {
                closeClientSocket(index);
                signOut(index);
                currentLogin = NULL;
                continue;
            }
        }
        else {
            while(1) {
                memset(done, '\0', sizeof(done));
                strcpy(done,"Insert Username\n");
                if (searchAccount(username) == 2) {
                    memset(done, '\0', sizeof(done));
                    strcpy(done,"Account is logging\nInsert Username\n");
                }
                bytes_sent = send(conn_sock, done, strlen(done), 0);
                memset(username, '\0', sizeof(username));
                bytes_received = recv(conn_sock, username, sizeof(recv_data), 0);
                if (bytes_received <= 0) {
                    // Đóng kết nối và thực hiện các công việc cần thiết
                    closeClientSocket(index);
                    printf("Connection closed by client.\n");
                    return ;
                }
                username[bytes_received] = '\0';
                printf("%s %ld %d\n",username, strlen(username), searchAccount(username));
                if (searchAccount(username) == 1) break;
            }
            memset(done, '\0', sizeof(done));
            strcpy(done,"Insert password \n");

            bytes_sent = send(conn_sock, done, strlen(done), 0);
            do {
                if (bytes_sent < 0) {
                    perror("Error: ");
                    close(conn_sock);
                    return ;
                }
                memset(buff, '\0', sizeof(buff));
                bytes_received = recv(conn_sock, buff, sizeof(buff), 0);
                if (checkValid(buff) == 0) {
                    strcpy(done,"=))\n");
                    printf("%s\n",buff );
                    bytes_sent = send(conn_sock, done, strlen(done), 0);
                    continue;
                    if (bytes_sent < 0) {
                        perror("Error: ");
                        close(conn_sock);
                        break;
                    }
                }
                strcpy(password, buff);
                sem_wait(&semaphore);
                loadAccounts();
                struct Account *current = head;
                while (strcmp(current->username, username) != 0) {
                    current = current->next;
                }
                if (current->isLogin != 0) {
                    sem_post(&semaphore);
                    break;
                }
                if (current->status == 0) {
                    sem_post(&semaphore);
                    break;
                }
                if (current->isLogin == 0 && strcmp(current->password, password) == 0) {
                    if (current->status == 1 && current->isLogin == 0) {
                        current->wrongPasswordCount = 0;
                        currentLogin = current;
                        current->isLogin = index;
                        if (currentLogin != NULL) printf("Oke\n");
                        printf("%s %s %d %d %lu\n", current->username, current->password, current->status, current->wrongPasswordCount, current->isLogin);
                        saveAccounts();
                        sem_post(&semaphore);
                        break;
                    } else {
                        memset(done, '\0', sizeof(done));
                        strcpy(done,"Account is blocked\n");
                        bytes_sent = send(conn_sock, done, strlen(done), 0);
                        if (bytes_sent < 0) {
                            perror("Error: ");
                            closeClientSocket(index);
                            sem_post(&semaphore);
                            break ;
                        }
                        sem_post(&semaphore);
                        break;
                    }
                } else {
                    current->wrongPasswordCount++; 
                    if (current->wrongPasswordCount == 3) {
                        current->status = 0; 
                        current->wrongPasswordCount = 0;
                        strcpy(done,"Password is incorrect. Account is blocked\n");
                        bytes_sent = send(conn_sock, done, strlen(done), 0);
                        if (bytes_sent < 0) {
                            perror("Error: ");
                            closeClientSocket(index);
                            sem_post(&semaphore);
                            break ;
                        }
                        saveAccounts();
                        sem_post(&semaphore);
                        break;
                    } else {
                        memset(done, '\0', sizeof(done));
                        strcpy(done,"Password is incorrect\n");
                        bytes_sent = send(conn_sock, done, strlen(done), 0);
                        if (bytes_sent < 0) {
                            perror("Error: ");
                            closeClientSocket(index);
                            sem_post(&semaphore);
                            break ;
                        }
                        saveAccounts();
                        sem_post(&semaphore);
                        continue;
                    }
                }
            } while (strcmp(buff,"\0") != 0 );
        }
    }
}
void loadAccounts() {
    file = fopen("account.txt", "r");
    if (file == NULL) {
        printf("Not found.\n");
        return;
    }

    char username[50];
    char password[50];
    int status;
    int wrongPasswordCount;
    unsigned long isLogin;
    head = NULL;
    while (fscanf(file, "%s %s %d %d %lu", username, password, &status, &wrongPasswordCount, &isLogin) != EOF) {
        struct Account *newAccount = (struct Account *)malloc(sizeof(struct Account));
        strcpy(newAccount->username, username);
        strcpy(newAccount->password, password);
        newAccount->status = status;
        newAccount->wrongPasswordCount = wrongPasswordCount;
        newAccount->isLogin = isLogin;
        newAccount->next = NULL;
        if (head == NULL) {
            head = newAccount;
        } else {
            struct Account *current = head;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = newAccount;
        }
    }
    fclose(file);
}


int searchAccount(const char *username)
{
    loadAccounts();
    struct Account *current = head;
    while (current != NULL)
    {
        if (strcmp(current->username, username) == 0)
        {
            if (current->isLogin != 0)
            {
                return 2; // Account is already logged in
            }
            if (current->status == 1 && current->isLogin == 0)
            {
                return 1; // Account is active and not logged in
            }
            else
            {
                return 0; // Account is not active
            }
        }
        current = current->next;
    }
    return -1; // Account not found
}

int checkValid(const char *input)
{
    for (int i = 0; input[i] != '\0'; i++)
    {
        if (!isalnum(input[i]))
        {
            return 0; // Non-alphanumeric character found
        }
    }
    return 1; // String is valid
}

void saveAccounts()
{
    FILE *file = fopen("account.txt", "w");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    struct Account *current = head;
    while (current != NULL)
    {
        fprintf(file, "%s %s %d %d %lu\n", current->username, current->password, current->status, current->wrongPasswordCount, current->isLogin);
        current = current->next;
    }

    fclose(file);
}

void signOut(unsigned long pid)
{
    struct Account *current = head;
    while (current != NULL)
    {
        if (current->isLogin == pid)
        {
            current->isLogin = 0;
        }
        current = current->next;
    }
    saveAccounts();
}

void signal_handler(int signal_num)
{
    loadAccounts();
    struct Account *current = head;
    while (current != NULL)
    {
        if (current->isLogin != 0)
        {
            current->isLogin = 0;
        }
        current = current->next;
    }
    saveAccounts();
}

int sendData(int s, char *buff, int size, int flags)
{
    int total_sent = 0;
    int bytes_sent;

    while (total_sent < size)
    {
        bytes_sent = send(s, buff + total_sent, size - total_sent, flags);

        if (bytes_sent <= 0)
        {
            perror("Error sending data");
            break;
        }

        total_sent += bytes_sent;
    }

    // Check if all expected bytes were sent
    if (total_sent != size)
    {
        perror("Error sending data: Incomplete send");
    }

    return total_sent;
}

int receiveData(int s, char *buff, int size, int flags)
{
    int bytes_received = recv(s, buff, size, flags);
    if (bytes_received <= 0)
    {
        if (bytes_received == 0)
        {
            // Connection closed by the client
            printf("Connection closed by client\n");
        }
        else
        {
            perror("Error receiving data");
        }
    }

    // Check if all expected bytes were received
    if (bytes_received != size)
    {
        perror("Error receiving data: Incomplete receive");
    }

    return bytes_received;
}
