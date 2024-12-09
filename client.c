#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define LG_MESSAGE 256

int socketDialogue;

void recvMessage() {
    char messageRecu[LG_MESSAGE];
    while (1) {
        if (recv(socketDialogue, messageRecu, LG_MESSAGE, 0) == -1) {
            perror("recv");
            close(socketDialogue);
            exit(-5);
        }
        printf("%s\n", messageRecu);
    }
}

void sendMessage() {
    char messageEnvoi[LG_MESSAGE];
    while (1) {
        fgets(messageEnvoi, LG_MESSAGE, stdin);
        if (send(socketDialogue, messageEnvoi, strlen(messageEnvoi), 0) == -1) {
            perror("send");
            close(socketDialogue);
            exit(-6);
        }
    }
}


int main(int argc, char *argv[]) {
    // Déclaration des variables et des structures
    int socketDialogue;
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
    socketDialogue = socket(AF_INET, SOCK_STREAM, 0);

    if (socketDialogue < 0) {
        perror("Erreur en création de la socket...");
        exit(-1);
    }
    printf("Socket créée! (%d)\n", socketDialogue);

    // Remplissage de sockaddrDistant
    longueurAdresse = sizeof(sockaddrDistant);
    memset(&sockaddrDistant, 0, longueurAdresse);

    sockaddrDistant.sin_family = AF_INET;
    sockaddrDistant.sin_port = htons(port_dest);
    sockaddrDistant.sin_addr.s_addr = inet_addr(ip_dest);

    // Connexion au serveur distant
    if (connect(socketDialogue, (struct sockaddr *)&sockaddrDistant, longueurAdresse) == -1) {
        perror("Erreur de connection avec le serveur distant...");
        close(socketDialogue);
        exit(-2);
    }
    printf("Connexion au serveur %s:%d réussie!\n", ip_dest, port_dest);

    pthread_t recvMessageThread;
    pthread_create(&recvMessageThread, NULL, (void*)recvMessage, NULL);

    pthread_t sendMessageThread;
    pthread_create(&sendMessageThread, NULL, (void*)sendMessage, NULL);

    pthread_join(recvMessageThread, NULL);
    pthread_join(sendMessageThread, NULL);

    close(socketDialogue);
    return 0;
}