#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define LG_MESSAGE 256
#define PORT_DE_BASE 2000

// Fonction de gestion d'un client
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

int main() {
    int socketEcoute, socketDialogue;
    struct sockaddr_in pointDeRencontreLocal, pointDeRencontreDistant;
    socklen_t longueurAdresse;
    pthread_t threadClient;
    int port = PORT_DE_BASE;  // Port de départ, le serveur commence ici

    // Crée un socket de communication
    socketEcoute = socket(AF_INET, SOCK_STREAM, 0);
    if (socketEcoute < 0) {
        perror("socket");
        exit(-1);
    }

    // Remplissage de sockaddrLocal
    memset(&pointDeRencontreLocal, 0, sizeof(pointDeRencontreLocal));
    pointDeRencontreLocal.sin_family = AF_INET;
    pointDeRencontreLocal.sin_addr.s_addr = htonl(INADDR_ANY);  // Accepter toutes les adresses IP
    pointDeRencontreLocal.sin_port = htons(port);

    // Demande d'attachement local de la socket
    if (bind(socketEcoute, (struct sockaddr *)&pointDeRencontreLocal, sizeof(pointDeRencontreLocal)) < 0) {
        perror("bind");
        close(socketEcoute);
        exit(-2);
    }

    printf("Serveur démarré sur le port %d\n", port);

    // Fixe la taille de la file d'attente à 5
    if (listen(socketEcoute, 5) < 0) {
        perror("listen");
        close(socketEcoute);
        exit(-3);
    }

    while (1) {
        // Attente d'une connexion
        socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
        if (socketDialogue < 0) {
            perror("accept");
            close(socketDialogue);
            continue;
        }

        // Créer un thread pour gérer chaque client
        printf("Connexion acceptée, affectation du port %d pour ce client...\n", port + 1);
        if (pthread_create(&threadClient, NULL, gestionClient, (void *)&socketDialogue) != 0) {
            perror("Erreur de création de thread");
        } else {
            pthread_detach(threadClient);  // Détacher le thread pour qu'il se termine proprement
        }

        // Augmenter le port pour la prochaine connexion
        port++;
        pointDeRencontreLocal.sin_port = htons(port);  // Mettre à jour le port d'écoute pour la prochaine connexion
    }

    close(socketEcoute);
    return 0;
}
