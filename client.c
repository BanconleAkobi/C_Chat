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

void removeNewlines(char *s) {
    char *newline = strpbrk(s, "\r\n");
    if (newline) {
        *newline = '\0';
    }
}

void recvMessage() {
    while (1) {
        char messageRecu[LG_MESSAGE] = {0};
        if (recv(socketDialogue, messageRecu, LG_MESSAGE, 0) == -1) {
            perror("recv");
            close(socketDialogue);
            exit(-5);
        }
        printf("%s\n", messageRecu);
    }
}

void sendMessage() {
    while (1) {
        char messageEnvoi[LG_MESSAGE] = {0};
        fgets(messageEnvoi, LG_MESSAGE, stdin);
        removeNewlines(messageEnvoi);

        // Get the message length
        int messageLength = strlen(messageEnvoi);
        if (messageLength > 0) {
            // Send the length first
            if (send(socketDialogue, &messageLength, sizeof(int), 0) == -1) {
                perror("send length");
                close(socketDialogue);
                exit(-6);
            }
            // Send the actual message
            if (send(socketDialogue, messageEnvoi, messageLength, 0) == -1) {
                perror("send message");
                close(socketDialogue);
                exit(-6);
            }
        }
    }
}


int main(int argc, char *argv[]) {
    // Déclaration des variables et des structures
    struct sockaddr_in sockaddrDistant;
    socklen_t longueurAdresse;

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

    char username[LG_MESSAGE];
    printf("Entrez votre nom d'utilisateur: ");
    fgets(username, LG_MESSAGE, stdin);
    removeNewlines(username);

    
    char response[LG_MESSAGE] = {0};
    do {
        // Send username
        send(socketDialogue, username, LG_MESSAGE, 0);

        // Receive response
        if (recv(socketDialogue, response, LG_MESSAGE, 0) == -1) {
            perror("recv");
            close(socketDialogue);
            exit(-5);
        }
    } while (strcmp(response, "USERNAME_OK") != 0);

    pthread_t recvMessageThread;
    pthread_create(&recvMessageThread, NULL, (void*)recvMessage, NULL);

    pthread_t sendMessageThread;
    pthread_create(&sendMessageThread, NULL, (void*)sendMessage, NULL);

    pthread_join(recvMessageThread, NULL);
    pthread_join(sendMessageThread, NULL);

    close(socketDialogue);
    return 0;
}