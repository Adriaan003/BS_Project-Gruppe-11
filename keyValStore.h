
#ifndef BSPA23_KEYVALSTORE_H
#define BSPA23_KEYVALSTORE_H

void setKey(char* key, char* value);
char *getKey(char* key);
void delKey(char *key, int clientSocket);
#endif //BSPA23_KEYVALSTORE_H
