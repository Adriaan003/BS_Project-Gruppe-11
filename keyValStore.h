
#ifndef BSPA23_KEYVALSTORE_H
#define BSPA23_KEYVALSTORE_H
typedef struct Data Data;

void setKey(char* key, char* value, Data *sharedData);
char *getKey(char* key, Data *sharedData);
void delKey(char *key, int clientSocket, Data *sharedData);
#endif //BSPA23_KEYVALSTORE_H
