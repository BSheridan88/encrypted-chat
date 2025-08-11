#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>

int p2;

unsigned char send_message[256];
unsigned char receive_message[256];
pthread_mutex_t lock;
int send_flag = 0;
int exit_flag = 0;

void xor(char *msg,size_t msg_len, const char *key,size_t key_len) {
    size_t key_index = 0;
    for(int i=0;i<msg_len;i++) {
        msg[i] ^= key[key_index % key_len];
        key_index++;
    }
}

void *send_msg(void *arg) { //type cast all vars
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
    unsigned char *encrypted_msg = receive_message;
    int receive_sock = *(int *)arg;
    char key[256];


    size_t encrypted_len = sizeof(receive_message);


    while (exit_flag != 1) {
        size_t data_recieved = recv(receive_sock,encrypted_msg,encrypted_len, 0);
       //while ((data_recieved = recv(receive_sock,encrypted_msg,encrypted_len,0)) > 0 ) {
        if (data_recieved > 0){
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
    //set up socket
    char *ip = " ";//ip
    int port = 4444;
    struct sockaddr_in client;
    client.sin_addr.s_addr = inet_addr(ip);
    client.sin_port = htons(port);
    client.sin_family = AF_INET;
    p2 = socket(AF_INET, SOCK_STREAM, 0);
    socklen_t len = sizeof(client);
    printf("[-] Connecting to chatroom...\n");
    if (connect(p2, (struct sockaddr *)&client, len) == -1) {
        return -1;
    }
    printf("[+] Connected to chatroom \n");


    //
    //create and join thread
    pthread_t send, receive;
    pthread_mutex_init(&lock,NULL);

    pthread_create(&send,NULL,send_msg,(void *)&p2);//args passed in last , //NULL for test cause fuck it
    pthread_create(&receive,NULL,receive_msg,(void *)&p2);//args passed in last ,//NULL for test cause fuck it


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
                exit_flag = 1;
                break;
            }
        }
    }
        pthread_join(send,NULL);
        pthread_join(receive,NULL);

    return 0;
}
