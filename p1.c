#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
int p1, join;

void xor(char *msg,size_t msg_len, const char *key, size_t key_len){
    size_t key_index = 0;
    for(int i=0;i<msg_len;i++) {
        msg[i] ^= key[key_index % key_len];
        key_index++;
    }
}

void *send_msg(void *message_s) { //type case every variable
    unsigned char *msg = message_s;

   // unsigned char msg[256];
    unsigned char key[256];


    printf("Input message: \n");
    fgets(msg,sizeof(msg),stdin);
    printf("Input key: \n");
    fgets(key,sizeof(key),stdin);

    msg[strcspn((char *)msg, "\n")] = '\0';
    key[strcspn((char *)key, "\n")] = '\0';

    size_t msg_len = strlen(msg);
    size_t key_len = strlen(key);

    unsigned char encrypted[256];


    xor(msg,msg_len,key,key_len);
    for (int i = 0; i < msg_len; i++) {
        encrypted[i] = msg[i];
    }


    size_t sent = send(p1,encrypted,msg_len,0);
    if (sent == -1) {
        printf("[x] Message failed to send \n");
    }
    return NULL;
}
void *receive_msg(void *message_r) {
    unsigned char *encrypted_msg = message_r;
   //unsigned char encrypted_msg[256];
    char key[256];


    size_t encrypted_len = sizeof(encrypted_msg);



    size_t data_recieved;
    while ((data_recieved = recv(join,encrypted_msg,encrypted_len,0)) > 0 ) {
        printf("Input key: \n");
        fgets(key,sizeof(key),stdin);
        key[strcspn(key, "\n")] = '\0';
        size_t key_len = strlen(key);

        xor(encrypted_msg,data_recieved,key,key_len);

        unsigned char decrypted_msg[256];
        for (int i = 0; i < data_recieved; i++) {
            decrypted_msg[i] = encrypted_msg[i];
        }
        decrypted_msg[data_recieved] = '\0';
        printf("Anonymous: %s\n", (char *)decrypted_msg);
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
    unsigned char message_s[256];
    unsigned char message_r[256];
    ///create and join thread
    pthread_t send, receive;
    pthread_create(&send,NULL,send_msg,(void *)&message_s);//args passed in last ,
    pthread_create(&receive,NULL,receive_msg,(void *) &message_r);//args passed in last ,

    pthread_join(send_msg,NULL);
    pthread_join(receive_msg,NULL);
}
//watch video on threads again
//confused on the concept of passing vars through threads cause i need user input in both threads maybe at the same time
//Issue: might need to type your message while typing key to unlock message so thats gonna have to be addressed too
