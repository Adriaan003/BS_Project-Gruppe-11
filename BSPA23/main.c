#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#define SERVER_PORT 5678


void hello(int clientSocket) {
    char *msg = "Hello, World!\n";
    send(clientSocket, msg, strlen(msg), 0);
}


int main() {
    int servSocket, clientSocket[3], client_len, status;
    struct sockaddr_in server, client;
    pid_t pid[3];

    int value;

    // Festlegung eines neues Sockets, der:
    // - über IPv4-Adressen kommuniziert
    // - für die Verwendung von TCP konfiguriert ist
    // - dem Betriebsystem die Wahl des Protokolltyps überlässt

    servSocket = socket(AF_INET, SOCK_STREAM, 0);

    // Diese Zeilen initialisieren den Server, um auf Verbindungen von Clients zu warten.
    // 1. Adressfamilie = AF_INET (IPv4 basierte Kommunikation)
    // 2. Server IP-Adresse = INADDR_ANY (erlaubt alle IP-Adressen des Rechners für das Socket)
    // 3. Festlegung des Ports, auf dem der Server auf eingehende Verbindungen warten wird
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(SERVER_PORT);

    //  HIER:
    //  Sock - Unser File-descriptor, der an die Adresse gebunden werden soll
    //  &server - Die Adresse im Netzwerk, an die das Socket gebunden werden soll
    //  sizeof(server) - Die größe des zweiten Parameters (Struktur server)
    if (0 > bind(servSocket, (struct sockaddr *) &server, sizeof(server))) {
        perror("Fehlerfall: \n");
        exit(EXIT_FAILURE);

    }


    // Wartemodus:
    // Sock - unser Server Socket, auf dem die Verbindung stattfinden soll
    // 3 - Anzahl der Clients in der Warteschlange

    if (0 > listen(servSocket, 3)) {
        perror("socket konnte nicht listen gesetzt werden\n");
        exit(EXIT_FAILURE);
    }

    printf("Server wartet auf Port %d...\n", SERVER_PORT);

    client_len = sizeof(client);

    for (int i = 0; i < 3; ++i) { //Erzeugung Kindprozesse
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
                if (clientSocket[j] != 0) {
                    //  Falls eine Verbindung entsteht, wird diese mit der Methode accept() akzeptiert
                    //  Das (leere) Objekt Client wird mit Information des Clients gefüllt (IP-Adresse & Port)
                    //  Unser Rückgabewert gibt uns einen NEUEN fD, den wir für die Verbindung mit DIESEM Client nutzen können
                    clientSocket[j] = accept(servSocket, (struct sockaddr *) &client, &client_len);

                    char quit = ' ';
                    while (quit != 'y') {
                        char *buffer = malloc(100 * sizeof(char)); // Dynamische Platzfreihaltung (Array[100])

                        sprintf(buffer, "Meine PID: %i\n", getpid());
                        // Die Nachricht im Buffer wird an Client Socket verschickt
                        send(clientSocket[j], buffer, strlen(buffer), 0);


                        sprintf(buffer, "Bitte 1 fuer hallo oder 2 fuer exit eingeben\n");
                        send(clientSocket[j], buffer, strlen(buffer), 0);


                        //Empfang der Daten vom Client. Die Daten werden im buffer gespeichert
                        if (read(clientSocket[j], buffer, sizeof(int)) < 0) {
                            perror("Es konnten keine Daten empfangt werden.");
                            exit(EXIT_FAILURE);
                        }

                        //Fun "strtol" Konvertiert String into Long
                        value = strtol(buffer, NULL, 0);

                        //value = ntohl(value);

                        switch (value) {
                            case 1:
                                hello(clientSocket[j]);
                                break;
                            case 2:
                                sprintf(buffer, "y für Exit\n");
                                send(clientSocket[j], buffer, strlen(buffer), 0);
                                // scanf(" %c", &exit);
                                read(clientSocket[j], &quit, sizeof(quit));
                                break;
                            default:
                                sprintf(buffer, "Falsche Eingabe! 1 für Text, 2 für exit\n");
                                send(clientSocket[j], buffer, strlen(buffer), 0);
                                break;
                        }
                        free(buffer);
                    }
                    exit(0);
                }
            }
        }
    }

// Hier chillt der Vater
    for (int i = 0; i < 3; ++i) {
        waitpid(pid[i], NULL, 0);
        close(clientSocket[i]);
    }


    close(servSocket);


    return 0;


}
