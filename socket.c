#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "socket.h"

#define SERVER_PORT 5678



int setServSocket() {

    int servSocket;
    struct sockaddr_in server;
// Festlegung eines neues Sockets, der:
// - über IPv4-Adressen kommuniziert
// - für die Verwendung von TCP konfiguriert ist
// - dem Betriebsystem die Wahl des Protokolltyps überlässt

    servSocket = socket(AF_INET, SOCK_STREAM, 0);

// Diese Zeilen initialisieren den Server, um auf Verbindungen von Clients zu warten.
// 1. Adressfamilie = AF_INET (IPv4 basierte Kommunikation)
// 2. Server IP-Adresse = INADDR_ANY (erlaubt den Server auf alle IP-Adressen des Rechners anzunehmen)
// 3. Festlegung des Ports, auf dem der Server auf eingehende Verbindungen warten wird
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(SERVER_PORT);

// Optional! Methode setsockopt ermöglicht uns den Port freizugeben, bevor das Programm geschloßen wird.
    int optval = 1;
    setsockopt(servSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

//  HIER:
//  servSocket - Unser File-descriptor, der an die Adresse gebunden werden soll
//  &server - Enthält den Port und die Adresse im Netzwerk, an die das Socket gebunden werden soll
//  sizeof(server) - Die größe des zweiten Parameters (Struktur server)


    if (0 > bind(servSocket, (struct sockaddr *) &server, sizeof(server))) {
        perror("Fehlerfall: \n");
        exit(1);
    }

// Wartemodus:
// servSocket - unser Server Socket, auf dem die Verbindung stattfinden soll
// 3 - Anzahl der Clients in der Warteschlange

    if (0 > listen(servSocket, 3)) {
        perror("socket konnte nicht listen gesetzt werden\n");
        exit(1);
    }

    printf("Server wartet auf Port %d...\n", SERVER_PORT);
    return servSocket;
}

