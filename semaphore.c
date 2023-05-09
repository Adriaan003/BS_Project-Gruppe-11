#include <sys/sem.h>
#include "semaphore.h"
#include <stdlib.h>
#include <stdio.h>

struct sembuf enter, leave;

int createSemaphore()
{
    unsigned short marker[1];

    int sem_id;
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0644);
    if (sem_id == -1) {
        perror("Die Gruppe konnte nicht angelegt werden!");
        exit(1);
    }
    // Anschließend wird der Semaphor auf 1 gesetzt
    marker[0] = 1;
    semctl(sem_id, 1, SETALL, marker);  // alle Semaphore auf 1

    enter.sem_num = leave.sem_num = 0;  // Semaphor 0 in der Gruppe
    enter.sem_flg = leave.sem_flg = SEM_UNDO;
    enter.sem_op = -1; // blockieren, DOWN-Operation
    leave.sem_op = 1;   // freigeben, UP-Operation

    return sem_id;
}

void openSemaphore(int sem_id)
{
    semop(sem_id, &enter, 1); // Eintritt in kritischen Bereich
}

void closeSemaphore(int sem_id)
{
    semop(sem_id, &leave, 1); // Verlassen des kritischen Bereichs
}
