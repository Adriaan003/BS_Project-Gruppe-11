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
#include <mqueue.h>
#include <sys/msg.h>
#include "sub.h"

#define MAX_MSG_SIZE 256

#define NumberOfCHILDS 2


struct message {
    long mtype;
    char mtext[MAX_MSG_SIZE];
};

Data newData[50];
Data sharedStruct[NumberOfCHILDS][50];

void clientReadLine(int clientSocket, char *buffer) {
    if (read(clientSocket, buffer, BUFSIZ) < 0) {
        perror("Es konnten keine Daten empfangt werden.\t");
        exit(1);
    }
}


void runProgram(int *clientSocket,
                char *arg1, char *arg2, char *arg3,
                Data *sharedData, Data (*sharedSub)[50], char *buffer, char *sharedArg) {
    char set[3] = "SET";
    char get[3] = "GET";
    char del[3] = "DEL";
    char quit[4] = "QUIT";


    if (strncmp(arg1, set, strlen("SET")) == 0) {

        setKey(arg2, arg3, sharedData, sharedSub, clientSocket);


    } else if (strncmp(arg1, get, strlen("GET")) == 0) {
        *buffer = 0;

        buffer = getKey(arg2, sharedData);
        send(*clientSocket, buffer, strlen(buffer), 0);


    } else if (strncmp(arg1, del, strlen("DEL")) == 0) {

        delKey(arg2, *clientSocket, sharedData);


    } else if (strncmp(arg1, quit, strlen("QUIT")) == 0) {

        //buffer = "Client Socket wird geschloßen.\n";
        strcpy(sharedArg, "END");

        //send(clientSocket, buffer, strlen(buffer), 0);
        close(*clientSocket);


    }

}

int main() {
    int clientSocket[NumberOfCHILDS], client_len, shm_id, shm_id2, shm_id3, shm_id4, sem_id;
    struct sockaddr_in client;
    pid_t pid[NumberOfCHILDS];


    int servSocket = setServSocket();

    sem_id = createSemaphore();

    // Implementierung gemeinsamen Speicherplatzes
    // sharedData für Value storage

    Data *sharedData;

    shm_id = shmget(IPC_PRIVATE, sizeof(Data) * 50, IPC_CREAT | 0644);

    sharedData = (Data *) (struct Data *) shmat(shm_id, NULL, 0);

    for (int i = 0; i < 50; ++i) {
        sharedData[i] = newData[i];
    }


    //sharedSub für exklusive Benachrichtigungen
    Data (*sharedSub)[50]; // Declare sharedSub as a double pointer

    shm_id3 = shmget(IPC_PRIVATE, sizeof(Data) * NumberOfCHILDS * 50, IPC_CREAT | 0644);
    void *shared_mem_ptr = shmat(shm_id3, NULL, 0);
    sharedSub = (Data (*)[50]) shared_mem_ptr; // Cast the shared memory pointer to the correct type

    for (int i = 0; i < NumberOfCHILDS; ++i) {
        for (int keyV = 0; keyV < 50; ++keyV) {
            sharedSub[i][keyV] = sharedStruct[i][keyV];
        }
    }

//sharedClient für exklusive Benachrichtigungen


    shm_id4 = shmget(IPC_PRIVATE, sizeof(int) * NumberOfCHILDS, IPC_CREAT | 0644);
    int *sharedClient = shmat(shm_id4, NULL, 0);






    //SharedArg für BEG/END Befehle
    char *sharedArg;
    shm_id2 = shmget(IPC_PRIVATE, sizeof(char) * 6, IPC_CREAT | 0644);

    sharedArg = (char *) shmat(shm_id2, 0, 0);

    if (shm_id == -1 || shm_id2 == -1) {
        perror("Das Segment konnte nicht angelegt werden!");
        exit(1);
    }


    //Message queue


    key_t key;
    int mq;
    struct message msg;

    // unique key für shared message queue
    key = ftok("/path/to/keyfile", 'A');


    mq = msgget(key, 0666);
    if (mq == -1) {
        perror("msgget");
        exit(1);
    }


    for (int i = 0; i < NumberOfCHILDS; ++i) {
        pid[i] = fork();        // Erzeugung Kind-Prozesse
        if (pid[i] == -1) {
            perror("Kind konnte nicht erzeugt werden!");
            exit(1);
        }
        if (pid[i] == 0) {      // Verhinderung des exponentiellen Wachstums der Kind-Prozesse
            break;
        }

    }


    for (int j = 0; j < NumberOfCHILDS; ++j) {
        if (pid[j] == 0) {      // der Kind-Prozess startet hier seine Arbeit


            client_len = sizeof(client);
            //  Falls eine Verbindung entsteht, wird diese mit der Methode accept() akzeptiert
            //  Das (leere) Objekt Client wird mit Information des Clients gefüllt (IP-Adresse & Port)
            //  Unser Rückgabewert gibt uns einen NEUEN fD, den wir für die Verbindung mit DIESEM Client nutzen können

            sharedClient[j] = accept(servSocket, (struct sockaddr *) &client, &client_len);


            for (int i = 0; i < 1; ++i) {
                pid[i] = fork();        // Erzeugung Kind-Prozesse
                if (pid[i] == -1) {
                    perror("Kind konnte nicht erzeugt werden!");
                    exit(1);
                }
                if (pid[i] == 0) {      // Verhinderung des exponentiellen Wachstums der Kind-Prozesse
                    while (1) {         // Empfang der Nachricht aus MQ
                        int type = getpid() -4;
                        printf("RCV: %i\n",getpid());
                        printf("RCV: %i\n",type);

                        if (msgrcv(mq, &msg, sizeof(struct message),type , 0) == -1) {
                            perror("msgrcv");
                            exit(1);
                        }
                        printf("Success!");
                        char *msgg = malloc(MAX_MSG_SIZE);
                        char *msgg2 = malloc(MAX_MSG_SIZE);
                        strcpy(msgg2, "Value Changed : ");

                        strcpy(msgg, msg.mtext);
                        strcat(msgg, "\n");

                        write(sharedClient[j], msgg2, strlen(msgg2) );
                        write(sharedClient[j], msgg, strlen(msgg));
                        free(msgg);

                    }

                }

            }


            // Dynamische Platzreservierung  (Array[100])
            char *nachricht = malloc(100);

            sprintf(nachricht, "\nMeine PID: %i\n", getpid());
            // Die Nachricht im Buffer wird an Client Socket verschickt


            send(sharedClient[j], nachricht, strlen(nachricht), 0);
            free(nachricht);


            while (1) {

                char eeeeeeee[20] = "Ich komme hier an\n";
                char *buffer = malloc(BUFSIZ);


                char arg1[50], arg2[50], arg3[100];

                // Empfang der Daten vom Client. Die Daten werden im buffer gespeichert
                clientReadLine(sharedClient[j], buffer);


                sscanf(buffer, "%s %s %[^\n]", arg1, arg2, arg3);

                char beg[3] = "BEG";
                char end[3] = "END";
                char sub[3] = "SUB";


                if (strncmp(arg1, sub, strlen("SUB")) == 0) {
                    int num = strtol(arg2 + 3, NULL, 0);
                    int pID = getpid();


                    strncpy((char *) sharedSub[j][num].sub, sub, sizeof(sub));

                    sharedSub[j][num].pid = pID;


                    continue;

                }


                // die BEG/END exklusive Transaktionen beginnen hier.

                if (strncmp(arg1, beg, strlen("BEG")) == 0) {
                    strcpy(sharedArg, beg);
                    openSemaphore(sem_id);

                    while (1) {

                        clientReadLine(sharedClient[j], buffer);

                        sscanf(buffer, "%s %s %[^\n]", arg1, arg2, arg3);

                        if (strncmp(arg1, end, strlen("END")) == 0) {
                            strcpy(sharedArg, end);
                            send(sharedClient[j], eeeeeeee, strlen(eeeeeeee), 0);
                            closeSemaphore(sem_id);
                            break;
                        }

                        runProgram(sharedClient, arg1, arg2, arg3, sharedData, sharedSub, buffer, sharedArg);

                    }
                }


                //Standard Bedingungen, falls BEG/END nicht gewählt wurden

                if (strncmp(sharedArg, beg, strlen("BEG")) != 0) {

                    runProgram(sharedClient, arg1, arg2, arg3, sharedData, sharedSub, buffer, sharedArg);

                }
            }
        }
    }



// Vaterprozess wartet auf Kind-Prozesse
    if (pid[0] != 0) {
        for (int i = 0; i < NumberOfCHILDS; ++i) {
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
    close(mq);

    return 0;

}