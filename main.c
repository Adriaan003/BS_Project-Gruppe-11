#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "socket.h"
#include "keyValStore.h"


int main() {
    int clientSocket[3], client_len;
    struct sockaddr_in client;
    pid_t pid[3];

    int servSocket = setServSocket();


    for (int i = 0; i < 1; ++i) { //Erzeugung Kindprozesse
        pid[i] = fork();
        if (pid[i] == -1) {
            perror("Kind konnte nicht erzeugt werden!");
            exit(1);
        }
        if (pid[i] == 0) {// Verhinderung des exponentiellen Wachstums der Kindprozesse
            break;
        }

    }


    for (int k = 0; k < 3; ++k) {
        if (pid[k] == 0) { // Hier startet der Kindprozess. Multiclientfähig, da 3 Prozesse bereits erzeugt wurden
            for (int j = 0; j < 3; ++j) {

                // if (clientSocket[j] != 0) {

                client_len = sizeof(client);
                //  Falls eine Verbindung entsteht, wird diese mit der Methode accept() akzeptiert
                //  Das (leere) Objekt Client wird mit Information des Clients gefüllt (IP-Adresse & Port)
                //  Unser Rückgabewert gibt uns einen NEUEN fD, den wir für die Verbindung mit DIESEM Client nutzen können
                clientSocket[j] = accept(servSocket, (struct sockaddr *) &client, &client_len);
                // Dynamische Platzfreihaltung (Array[100])
                char *nachricht = malloc(100 * sizeof(char));

                sprintf(nachricht, "\nMeine PID: %i\n", getpid());
                // Die Nachricht im Buffer wird an Client Socket verschickt
                send(clientSocket[j], nachricht, strlen(nachricht), 0);


                while (1) {

                    char *buffer = malloc(BUFSIZ * sizeof(char));



                    //Empfang der Daten vom Client. Die Daten werden im buffer gespeichert
                    if (read(clientSocket[j], buffer, BUFSIZ) < 0) {
                        perror("Es konnten keine Daten empfangt werden.\t");
                        exit(1);
                    }

                    char arg1[50], arg2[50], arg3[100];
                    sscanf(buffer, "%s %s %[^\n]", arg1, arg2, arg3);


                    char set[3] = "SET";
                    char get[3] = "GET";
                    char del[3] = "DEL";
                    char quit[4] = "QUIT";


                    if (strncmp(arg1, set, strlen("SET")) == 0) {

                        setKey(arg2, arg3);

                    } else if (strncmp(arg1, get, strlen("GET")) == 0) {
                        *buffer = 0;

                        buffer = getKey(arg2);
                        send(clientSocket[j], buffer, strlen(buffer), 0);
                        //free(buffer);


                    } else if (strncmp(arg1, del, strlen("DEL")) == 0) {

                        delKey(arg2, clientSocket[j]);
                    } else if (strncmp(arg1, quit, strlen("QUIT")) == 0) {

                        buffer = "Client Socket wird geschloßen.\n";
                        send(clientSocket[j], buffer, strlen(buffer), 0);
                        break;


                    } else {
                        // Anweisungen, die ausgeführt werden, wenn weder Bedingung1 noch Bedingung2 wahr sind
                        buffer = "Ungültige Eingabe.\n";
                        send(clientSocket[j], buffer, strlen(buffer), 0);

                    }

                }
                exit(0);
                // }
            }
        }
    }

// Hier chillt der Vater
    for (int i = 0; i < 1; ++i) {
        waitpid(pid[i], NULL, 0);
        close(clientSocket[i]);

    }


    close(servSocket);


    return 0;


}
