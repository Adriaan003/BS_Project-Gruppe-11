#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "socket.h"
#include "keyValStore.h"
#include "semaphore.h"


typedef struct Data {
    char key[10];
    char value[50];
} Data;
Data newData[50];

int main() {
    int clientSocket[3], client_len, shm_id, shm_id2, sem_id;
    struct sockaddr_in client;
    pid_t pid[3];


    int servSocket = setServSocket();

    sem_id = createSemaphore();

    // Implementierung gemeinsamen Speicherplatzes
    // Shared Struct Data


    Data *sharedData;

    shm_id = shmget(IPC_PRIVATE, sizeof(Data) * 50, IPC_CREAT | 0644);

    sharedData = (struct Data *) shmat(shm_id, NULL, 0);
    for (int i = 0; i < 50; ++i) {
        sharedData[i] = newData[i];
    }


    char *sharedArg;
    shm_id2 = shmget(IPC_PRIVATE, sizeof(char) * 6, IPC_CREAT | 0644);


    sharedArg = (char *) shmat(shm_id2, 0, 0);


    if (shm_id == -1 || shm_id2 == -1) {
        perror("Das Segment konnte nicht angelegt werden!");
        exit(1);
    }


    for (int i = 0; i < 2; ++i) {
        pid[i] = fork();        // Erzeugung Kind-Prozesse
        if (pid[i] == -1) {
            perror("Kind konnte nicht erzeugt werden!");
            exit(1);
        }
        if (pid[i] == 0) {      // Verhinderung des exponentiellen Wachstums der Kind-Prozesse
            break;
        }

    }


    for (int j = 0; j < 2; ++j) {
        if (pid[j] == 0) {      // der Kind-Prozess startet hier seine Arbeit


            client_len = sizeof(client);
            //  Falls eine Verbindung entsteht, wird diese mit der Methode accept() akzeptiert
            //  Das (leere) Objekt Client wird mit Information des Clients gefüllt (IP-Adresse & Port)
            //  Unser Rückgabewert gibt uns einen NEUEN fD, den wir für die Verbindung mit DIESEM Client nutzen können
            clientSocket[j] = accept(servSocket, (struct sockaddr *) &client, &client_len);



            // Dynamische Platzreservierung  (Array[100])
            char *nachricht = malloc(100);

            sprintf(nachricht, "\nMeine PID: %i\n", getpid());
            // Die Nachricht im Buffer wird an Client Socket verschickt
            send(clientSocket[j], nachricht, strlen(nachricht), 0);
            free(nachricht);


            while (1) {

                char eeeeeeee[20] = "Ich komme hier an\n";
                char *buffer = malloc(BUFSIZ);


                char arg1[50], arg2[50], arg3[100];
                // Empfang der Daten vom Client. Die Daten werden im buffer gespeichert
                if (read(clientSocket[j], buffer, BUFSIZ) < 0) {
                    perror("Es konnten keine Daten empfangt werden.\t");
                    exit(1);
                }


                sscanf(buffer, "%s %s %[^\n]", arg1, arg2, arg3);

                char beg[3] = "BEG";
                char end[3] = "END";

                if (strncmp(arg1, beg, strlen("BEG")) == 0) {

                    strcpy(sharedArg, beg);
                }
                char set[3] = "SET";
                char get[3] = "GET";
                char del[3] = "DEL";
                char quit[4] = "QUIT";

                // die BEG/END exklusive Transaktionen beginnen hier.

                if (strncmp(arg1, beg, strlen("BEG")) == 0) {
                    openSemaphore(sem_id);

                    while (1) {
                        if (read(clientSocket[j], buffer, BUFSIZ) < 0) {
                            perror("Es konnten keine Daten empfangt werden.\t");
                            exit(1);
                        }
                        sscanf(buffer, "%s %s %[^\n]", arg1, arg2, arg3);

                        if (strncmp(arg1, set, strlen("SET")) == 0) {

                            setKey(arg2, arg3, sharedData);


                        } else if (strncmp(arg1, get, strlen("GET")) == 0) {
                            *buffer = 0;

                            buffer = getKey(arg2, sharedData);
                            send(clientSocket[j], buffer, strlen(buffer), 0);


                        } else if (strncmp(arg1, del, strlen("DEL")) == 0) {

                            delKey(arg2, clientSocket[j], sharedData);


                        } else if (strncmp(arg1, quit, strlen("QUIT")) == 0) {

                            //buffer = "Client Socket wird geschloßen.\n";
                            send(clientSocket[j], buffer, strlen(buffer), 0);
                            close(clientSocket[j]);


                        } else if (strncmp(arg1, end, strlen("END")) == 0) {
                            strcpy(sharedArg, end);
                            send(clientSocket[j], eeeeeeee, strlen(eeeeeeee), 0);
                            closeSemaphore(sem_id);
                            break;
                        }
                    }



                    // closeSemaphore(sem_id);
                }


                //Standard Bedingungen, falls BEG/END nicht gewählt wurden

                if (strncmp(sharedArg, beg, strlen("BEG")) != 0) {

                    if (strncmp(arg1, set, strlen("SET")) == 0) {
                        setKey(arg2, arg3, sharedData);

                    } else if (strncmp(arg1, get, strlen("GET")) == 0) {
                        *buffer = 0;

                        buffer = getKey(arg2, sharedData);
                        send(clientSocket[j], buffer, strlen(buffer), 0);


                    } else if (strncmp(arg1, del, strlen("DEL")) == 0) {

                        delKey(arg2, clientSocket[j], sharedData);


                    } else if (strncmp(arg1, quit, strlen("QUIT")) == 0) {

                        //buffer = "Client Socket wird geschloßen.\n";
                        send(clientSocket[j], buffer, strlen(buffer), 0);
                        close(clientSocket[j]);

                    } else {
                        continue;
                    }
                }
                // memset(&arg1, 0, sizeof(arg1));
                // closeSemaphore(sem_id);
            }
        }
    }



// Vaterprozess wartet auf Kind-Prozesse
    if (pid[0] != 0) {
        for (int i = 0; i < 2; ++i) {
            printf("Vater mit ID %i wartet auf weitere/n Kind-Prozesse.\n", getpid());
            waitpid(pid[i], NULL, 0);
        }
        printf("Vater geht.");
        close(servSocket);
    }


    shmdt(sharedData);
    shmctl(shm_id,
           IPC_RMID, 0);
    shmdt(sharedArg);
    shmctl(shm_id2,
           IPC_RMID, 0);
    semctl(sem_id,
           0, IPC_RMID);

    return 0;

}
