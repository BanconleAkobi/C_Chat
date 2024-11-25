#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 5000
#define LG_MESSAGE 256
#define LG_USERNAME 20

typedef struct s_message {
    char username[LG_USERNAME];
    char message[LG_MESSAGE];
} t_message;

typedef struct s_message_list {
    pthread_mutex_t mutex;
    t_message *messages;
    int size;
} t_message_list;

typedef struct s_thread_data {
    int socketDialogue;
    t_message_list *message_list;
} t_thread_data;

void formatAllMessages(t_message_list *message_list) {
    pthread_mutex_lock(&message_list->mutex);
    for (int i = 0; i < message_list->size; i++) {
        
    }
    pthread_mutex_unlock(&message_list->mutex);
}

void *thread_routine(void *data)
{
    t_thread_data *thread_data = (t_thread_data*)data;
    int socketDialogue = thread_data->socketDialogue;
    t_message_list *message_list = thread_data->message_list;
    
    
}

int main(int argc, char *argv[]) {
    // Déclaration des variables et des structures
    t_message_list message_list;
    message_list.size = 1;
    message_list.messages = (int*)malloc(sizeof(t_message) * message_list.size);
    message_list.messages[0].username[0] = 'Serveur';
    message_list.messages[0].message[0] = 'Bienvenue sur le chat !';

    int socketEcoute;
    struct sockaddr_in pointDeRencontreLocal;
    socklen_t longueurAdresse;

    int socketDialogue;
    struct sockaddr_in pointDeRencontreDistant;
    char messageRecu1[LG_MESSAGE];
    char messageEnvoi[LG_MESSAGE];

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

    while (1) {
        printf("Attente d'une demande de connexion (quitter avec Ctrl-C)\n\n");

        socketDialogue = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant, &longueurAdresse);
        if (socketDialogue < 0) {
            perror("accept");
            close(socketDialogue);
            close(socketEcoute);
            exit(-4);
        }

        // Création d'un thread pour gérer la communication
        pthread_t tid;
        t_thread_data data;
        data.socketDialogue = socketDialogue;
        data.message_list = &message_list;
        pthread_create(&tid, NULL, thread_routine, &data);
    }

    // Fermeture des sockets
    close(socketDialogue);
    close(socketEcoute);
    return 0;
}