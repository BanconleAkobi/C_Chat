#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LG_MESSAGE 256

int main(int argc, char *argv[]) {
    // Déclaration des variables et des structures
    int descripteurSocket;
    struct sockaddr_in sockaddrDistant;
    socklen_t longueurAdresse;

    char messageRecu[LG_MESSAGE];
    char messageEnvoi[LG_MESSAGE];

    char ip_dest[16];
    int port_dest;

    // Vérification des arguments de la ligne de commande
    if (argc > 1) {
        strncpy(ip_dest, argv[1], 16);
        sscanf(argv[2], "%d", &port_dest);
    } else {
        printf("USAGE : %s ip port\n", argv[0]);
        exit(-1);
    }

    // Création du socket
    descripteurSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (descripteurSocket < 0) {
        perror("Erreur en création de la socket...");
        exit(-1);
    }
    printf("Socket créée! (%d)\n", descripteurSocket);

    // Remplissage de sockaddrDistant
    longueurAdresse = sizeof(sockaddrDistant);
    memset(&sockaddrDistant, 0, longueurAdresse);

    sockaddrDistant.sin_family = AF_INET;
    sockaddrDistant.sin_port = htons(port_dest);
    sockaddrDistant.sin_addr.s_addr = inet_addr(ip_dest);

    // Connexion au serveur distant
    if (connect(descripteurSocket, (struct sockaddr *)&sockaddrDistant, longueurAdresse) == -1) {
        perror("Erreur de connection avec le serveur distant...");
        close(descripteurSocket);
        exit(-2);
    }
    printf("Connexion au serveur %s:%d réussie!\n", ip_dest, port_dest);

    // Boucle de communication...

    // Fermeture du socket
    close(descripteurSocket);
    return 0;
}