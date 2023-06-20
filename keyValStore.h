
#ifndef BSPA23_KEYVALSTORE_H
#define BSPA23_KEYVALSTORE_H


typedef struct {
    char sub[10];
    char value[50];
    int pid;
} Data;



void setKey(char *key, char *value, Data *sharedData, Data (*sharedSub)[50], int *sharedClient);
char *getKey(char* key, Data *sharedData);
void delKey(char *key, int clientSocket, Data *sharedData);
#endif //BSPA23_KEYVALSTORE_H
