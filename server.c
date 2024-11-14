#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>  // Inclure pthread

#define PORT 5000
#define LG_MESSAGE 256

// Fonction pour gérer la communication avec un client
void *gestionClient(void *socketDialoguePtr) {
    int socketDialogue = *((int *)socketDialoguePtr);  // Récupérer le descripteur de socket
    char messageRecu[LG_MESSAGE];
    char messageEnvoi[LG_MESSAGE];
    
    while (1) {
        memset(messageRecu, 0, LG_MESSAGE);
        
        // Lecture du message du client
        int n = read(socketDialogue, messageRecu, LG_MESSAGE - 1);
        if (n <= 0) {
            perror("Erreur de lecture ou client déconnecté");
            break;
        }
        printf("Message reçu: %s\n", messageRecu);
        
        // Envoi d'un message au client
        printf("Entrez un message à envoyer au client: ");
        fgets(messageEnvoi, LG_MESSAGE, stdin);
        write(socketDialogue, messageEnvoi, strlen(messageEnvoi));

        if (strncmp(messageRecu, "exit", 4) == 0) {
            break;
        }
    }

    close(socketDialogue);
    printf("Connexion avec un client terminée.\n");
    pthread_exit(NULL);  // Terminer le thread
}

int main(int argc, char *argv[]) {
    int socketEcoute;
    struct sockaddr_in pointDeRencontreLocal;
    socklen_t longueurAdresse;
    int socketDialogue;
    struct sockaddr_in pointDeRencontreDistant;

    // Crée un socket de communication
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0) {
        perror("socket");
        exit(-1);
    }
    printf("Socket créée avec succès ! (%d)\n", socketEcoute);

    // Remplissage de sockaddrDistant
    longueurAdresse = sizeof(pointDeRencontreLocal);
    memset(&pointDeRencontreLocal, 0x00, longueurAdresse);
    pointDeRencontreLocal.sin_family = AF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);
    pointDeRencontreLocal.sin_port = htons(PORT);

    // Demande d’attachement local de la socket
    if (bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, longueurAdresse) < 0) {
        perror("bind");
        exit(-2);
    }
    printf("Socket attachée avec succès !\n");

    // Fixe la taille de la file d’attente à 5
    if (listen(socketEcoute, 5) < 0) {
        perror("listen");
        exit(-3);
    }
    printf("Socket placée en écoute passive...\n");

    pthread_t threadClient;  // Variable pour stocker le thread

    while (1) {
        printf("Attente d'une demande de connexion (quitter avec Ctrl-C)\n\n");

        socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
        if (socketDialogue < 0) {
            perror("accept");
            close(socketDialogue);
            close(socketEcoute);
            exit(-4);
        }
        
        // Création d'un thread pour gérer le client
        if (pthread_create(&threadClient, NULL, gestionClient, (void *)&socketDialogue) != 0) {
            perror("Erreur de création de thread");
        } else {
            pthread_detach(threadClient);  // Détache le thread pour qu'il se termine proprement
        }
    }

    close(socketEcoute);
    return 0;
}
