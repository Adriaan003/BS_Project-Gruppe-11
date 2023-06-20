
#include "keyValStore.h"
#include "stdlib.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include "string.h"
#include <sys/msg.h>

#define NumberOfCHILDS 2
#define MAX_MSG_SIZE 256


struct message {
    long mtype;
    char mtext[MAX_MSG_SIZE];
};


void setKey(char *key, char *value, Data *sharedData, Data (*sharedSub)[50], int *sharedClient) {


    int num = strtol(key + 3, NULL, 0);

    char *message = malloc(50);
    char sub[3] = "SUB";


    strncpy(sharedData[num].sub, key, sizeof(sharedData[num].sub));
    strncpy(sharedData[num].value, value, sizeof(sharedData[num].value));


    for (int i = 0; i < NumberOfCHILDS; ++i) {

        if (strncmp(sharedSub[i][num].sub, sub, strlen("SUB")) == 0) {

            key_t key_;
            int mq;
            struct message msg;

            // unique key für shared message queue
            key_ = ftok("/path/to/keyfile", 'A');

            mq = msgget(key_, 0666);
            if (mq == -1) {
                perror("msgget");
                exit(1);
            }


            msg.mtype = sharedSub[i][num].pid;  // Message type
            strncpy(msg.mtext, sharedData[num].value, MAX_MSG_SIZE);
            printf("SND: %li\n", msg.mtype);
            // Sendung der Nachricht in die MQ
            if (msgsnd(mq, &msg, sizeof(struct message), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }



        }
    }


}

char *getKey(char *key, Data *sharedData) {

    char *message = malloc(50); // Dynamische Speicherzuweisung

    int num = strtol(key + 3, NULL, 0);
    if (sharedData[num].value[0] == 0) {
        sprintf(message, "GET:key%i:key_nonexistent\n", num);
        return message;
    }
    sprintf(message, "GET:key%i:%s\n", num, sharedData[num].value);

    printf("%s\n", message);

    return message;
}

void delKey(char *key, int clientSocket, Data *sharedData) {
    char *res = malloc(50); // Dynamische Speicherzuweisung
    int num = strtol(key + 3, NULL, 0);

    if (sharedData[num].value[0] == 0) {
        sprintf(res, "DEL:key%i:key_nonexistent\n", num);
        send(clientSocket, res, strlen(res), 0);
    } else {
        sharedData[num].value[0] = 0;
        sprintf(res, "DEL:key%i:key_deleted\n", num);
        send(clientSocket, res, strlen(res), 0);
    }


    //Alternative um die Zeichenkette komplett zu löschen.
    //memset(&newData[num], 0, sizeof(Data));

}

