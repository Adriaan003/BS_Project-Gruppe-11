
#include "keyValStore.h"
#include "stdlib.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include "string.h"


typedef struct Data {
    char key[50];
    char value[50];
} Data;

Data newData[10];


void setKey(char *key, char *value) {
    int num = strtol(key + 3, NULL, 0);


    strncpy(newData[num].key, key, sizeof(newData[num].key));
    strncpy(newData[num].value, value, sizeof(newData[num].value));

}

char *getKey(char *key) {

    char *res = malloc(50); // Dynamische Speicherzuweisung

    int num = strtol(key + 3, NULL, 0);
    if (newData[num].value[0] == 0) {
        sprintf(res, "GET:key%i:key_nonexistent\n",num);
        return res;
    }
    sprintf(res, "GET:key%i:%s\n", num, newData[num].value);
    printf("%s\n", res);

    return res;
}

void delKey(char *key, int clientSocket) {
    char *res = malloc(50); // Dynamische Speicherzuweisung
    int num = strtol(key + 3, NULL, 0);

    if (newData[num].value[0] == 0) {
        sprintf(res, "DEL:key%i:key_nonexistent\n",num);
        send(clientSocket, res, strlen(res), 0);
    }

    newData[num].value[0] = 0;

    //Alternative um die Zeichenkette komplett zu löschen.
    //memset(&newData[num], 0, sizeof(Data));




}


