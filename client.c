#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define LG_MESSAGE 256

int descripteurSocket;

void *recevoirMessages(void *arg) {
    char messageRecu[LG_MESSAGE];

    while (1) {
        memset(messageRecu, 0, LG_MESSAGE);
        int n = read(descripteurSocket, messageRecu, LG_MESSAGE - 1);
        if (n <= 0) {
            printf("Serveur déconnecté ou erreur de lecture\n");
            break;
        }
        printf("Message du serveur: %s\n", messageRecu);
    }

    pthread_exit(NULL);
}

void *envoyerMessages(void *arg) {
    char messageEnvoi[LG_MESSAGE];

    while (1) {
        printf("Entrez un message à envoyer: ");
        fgets(messageEnvoi, LG_MESSAGE, stdin);
        write(descripteurSocket, messageEnvoi, strlen(messageEnvoi));

        if (strncmp(messageEnvoi, "exit", 4) == 0) {
            printf("Déconnexion du serveur...\n");
            break;
        }
    }

    pthread_exit(NULL);
}

int main() {
    char ip_dest[] = "127.0.0.1";  // Adresse IP du serveur local
    int port_dest;

//initialisation du port de destination.
    port_dest =  2000; 

    struct sockaddr_in sockaddrDistant;
    socklen_t longueurAdresse;

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
        perror("Erreur de connexion avec le serveur distant...");
        close(descripteurSocket);
        exit(-2);
    }
    printf("Connexion au serveur %s:%d réussie!\n", ip_dest, port_dest);

    pthread_t threadReception, threadEnvoi;

    // Création des threads pour recevoir et envoyer des messages
    pthread_create(&threadReception, NULL, recevoirMessages, NULL);
    pthread_create(&threadEnvoi, NULL, envoyerMessages, NULL);

    // Attente de la fin des threads
    pthread_join(threadReception, NULL);
    pthread_join(threadEnvoi, NULL);

    // Fermeture du socket
    close(descripteurSocket);
    return 0;
}
