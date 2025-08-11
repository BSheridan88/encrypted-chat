#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

int p1, join;

unsigned char send_message[256];
unsigned char receive_message[256];
pthread_mutex_t lock;
int send_flag = 0;
int exit_flag = 0;



void xor(char *msg,size_t msg_len, const char *key, size_t key_len){
    size_t key_index = 0;
    for(int i=0;i<msg_len;i++) {
        msg[i] ^= key[key_index % key_len];
        key_index++;
    }
}

void *send_msg(void *arg) {
    unsigned char *msg = send_message;
    int send_sock = *(int *)arg;
    unsigned char key[256];

    while (exit_flag != 1) {
        pthread_mutex_lock(&lock);
        if (send_flag == 1) {
            printf("Input message: \n");
            fgets((char *)msg,256,stdin);
            printf("Input key: \n");
            fgets(key,sizeof(key),stdin);

            msg[strcspn((char *)msg, "\n")] = '\0';
            key[strcspn((char *)key, "\n")] = '\0';

            size_t msg_len = strlen((char *)msg);
            size_t key_len = strlen(key);

            unsigned char encrypted[256];

            memcpy(encrypted,msg,msg_len);
            xor(encrypted,msg_len,key,key_len);

            printf("Me:%s \n",(char *)msg);

            size_t sent = send(send_sock,encrypted,msg_len,0);
            send_flag = 0;

            if (sent == -1) {
                printf("[x] Message failed to send \n");
            }
        }
        pthread_mutex_unlock(&lock);
        usleep(100000);
    }
    return NULL;
}
void *receive_msg(void *arg) {
    unsigned char *encrypted_msg =receive_message;
    int receive_sock = *(int *)arg;
    char key[256];

    size_t encrypted_len = sizeof(encrypted_msg);

    size_t data_recieved;
    while (1) {
        data_recieved = recv(receive_sock,encrypted_msg,encrypted_len,0);
        if (data_recieved > 0) {
            printf("Input key: \n");
            fgets(key,sizeof(key),stdin);
            key[strcspn(key, "\n")] = '\0';
            size_t key_len = strlen(key);

            xor((char *)encrypted_msg,data_recieved,key,key_len);

            unsigned char decrypted_msg[256];
            for (int i = 0; i < data_recieved; i++) {
                decrypted_msg[i] = encrypted_msg[i];
            }
            decrypted_msg[data_recieved] = '\0';
            printf("Anonymous: %s\n", (char *)decrypted_msg);
        }
    }
    return NULL;
}
int main() {
    int port = 4444;
    struct sockaddr_in server;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    server.sin_family = AF_INET;
    p1 = socket(AF_INET, SOCK_STREAM, 0);

     int b = bind(p1, (struct sockaddr *)&server, sizeof(server));
    if (b < 0) {
        exit(1);
    }
    listen(p1, 1);

    printf("[-] Connecting to chatroom...\n");
    socklen_t len = sizeof(server);
    join = accept(p1, (struct sockaddr *)&server, &len);
    if (join < 0) {
        printf("[+] Connection failed \n");
    }
    printf("[+] Connection accepted \n");
    ///create and join thread
    pthread_t send, receive;
    pthread_create(&send,NULL,send_msg,(void *)&join);//args passed in last ,
    pthread_create(&receive,NULL,receive_msg,(void *)&join);//args passed in last ,

    int stdin_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, stdin_flags | O_NONBLOCK);

    while (1) {
        char input[20];
        printf("Type ` to send a message or exit to EXIT \n");
        if (fgets(input,sizeof(input),stdin)!= NULL) {
            input[strcspn(input, "\n")] = '\0';
            if (strcmp(input,"`") == 0) {
                pthread_mutex_lock(&lock);
                send_flag = 1;
                pthread_mutex_unlock(&lock);
            }else if (strcmp(input,"exit") == 0) {
                // exit_flag = 1;
                break;
            }
        }
    }

    pthread_join(send,NULL);
    pthread_join(receive,NULL);
}
//watch video on threads again
//confused on the concept of passing vars through threads cause i need user input in both threads maybe at the same time
//Issue: might need to type your message while typing key to unlock message so thats gonna have to be addressed too
