
#include "keyValStore.h"
#include "stdlib.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include "string.h"


typedef struct Data {
    char key[10];
    char value[50];
} Data;

Data newData[50];



// Semaphor implementierung
void openSemaphor()
{

}




void setKey(char *key, char *value, Data *sharedData) {


    int num = strtol(key + 3, NULL, 0);


    strncpy(sharedData[num].key, key, sizeof(sharedData[num].key));
    strncpy(sharedData[num].value, value, sizeof(sharedData[num].value));

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
        sprintf(res, "DEL:key%i:key_nonexistent\n",num);
        send(clientSocket, res, strlen(res), 0);
    } else{
        sharedData[num].value[0] = 0;
        sprintf(res, "DEL:key%i:key_deleted\n",num);
        send(clientSocket, res, strlen(res), 0);
    }


    //Alternative um die Zeichenkette komplett zu löschen.
    //memset(&newData[num], 0, sizeof(Data));

}


