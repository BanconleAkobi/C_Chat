#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 5000
#define LG_MESSAGE 2000
#define LG_USERNAME 20
#define MAX_CLIENTS 5

typedef struct s_message {
    char username[LG_USERNAME];
    char message[LG_MESSAGE];
} t_message;

typedef struct s_message_list {
    pthread_mutex_t mutex;
    t_message *messages;
    int size;
} t_message_list;

typedef struct s_client_list {
    pthread_mutex_t mutex;
    int sockets[MAX_CLIENTS];
    int count;
} t_client_list;

t_client_list client_list;

typedef struct s_thread_data {
    int socketDialogue;
    t_message_list *message_list;
} t_thread_data;

void formatMessage(t_message_list *message_list, int index, char *buffer) {
    pthread_mutex_lock(&message_list->mutex);
    if (index < message_list->size, index >= 0) {
        sprintf(buffer, "%s : %s", message_list->messages[index].username, message_list->messages[index].message);
    } else {
        buffer[0] = '\0';
    }
    pthread_mutex_unlock(&message_list->mutex);
}

void addMessageToList(t_message_list *message_list, char *username, char *message) {
    pthread_mutex_lock(&message_list->mutex);
    t_message *newMessages = realloc(message_list->messages, sizeof(t_message) * (message_list->size + 1));
    if (newMessages == NULL) {
        perror("realloc");
        pthread_mutex_unlock(&message_list->mutex);
        return;
    }
    message_list->messages = newMessages;

    strncpy(message_list->messages[message_list->size].username, username, LG_USERNAME - 1);
    message_list->messages[message_list->size].username[LG_USERNAME - 1] = '\0';
    strncpy(message_list->messages[message_list->size].message, message, LG_MESSAGE - 1);
    message_list->messages[message_list->size].message[LG_MESSAGE - 1] = '\n';
    message_list->size++;

    pthread_mutex_unlock(&message_list->mutex);
}

int verifyUsername(char *username) {
    // Remove trailing newline
    if (username[strlen(username) - 1] == '\n') {
        username[strlen(username) - 1] = '\0';
    }
    if (strlen(username) <= 0 && strlen(username) >= LG_USERNAME) {
        return 0;
    }
    return 1;
}

void broadcastMessages(t_message_list *message_list, int count, int originSocket) {
    pthread_mutex_lock(&client_list.mutex);
    char buffer[LG_MESSAGE];
    for (int i = 0; i < client_list.count; i++) {
        int clientSocket = client_list.sockets[i];
        for (int j = message_list->size - count; j < message_list->size; j++) {
            if (j >= 0 && clientSocket != originSocket) {
                formatMessage(message_list, message_list->size-1, buffer);
                if (write(clientSocket, buffer, strlen(buffer)) < 0) {
                    perror("broadcast write");
                }
            }
        }
    }
    pthread_mutex_unlock(&client_list.mutex);
}

void addClientToList(int clientSocket) {
    pthread_mutex_lock(&client_list.mutex);
    if (client_list.count < MAX_CLIENTS) {
        client_list.sockets[client_list.count++] = clientSocket;
    }
    pthread_mutex_unlock(&client_list.mutex);
}

void removeClientFromList(int clientSocket) {
    pthread_mutex_lock(&client_list.mutex);
    for (int i = 0; i < client_list.count; i++) {
        if (client_list.sockets[i] == clientSocket) {
            client_list.sockets[i] = client_list.sockets[--client_list.count];
            break;
        }
    }
    pthread_mutex_unlock(&client_list.mutex);
}

void *thread_routine(void *data)
{
    t_thread_data *thread_data = (t_thread_data*)data;
    int socketDialogue = thread_data->socketDialogue;
    t_message_list *message_list = thread_data->message_list;
    
    char messageRecu[LG_MESSAGE];
    char messageEnvoi[LG_MESSAGE];
    char buffer[LG_MESSAGE];
    char empty[LG_MESSAGE];
    int justLoggedIn = 1;;
    char username[LG_USERNAME] = "";

    // Réception d'un nom d'utilisateur
    do {
        if (recv(socketDialogue, messageRecu, LG_MESSAGE, 0) == -1) {
            perror("recv");
            close(socketDialogue);
            exit(-5);
        }

        if (verifyUsername(messageRecu) == 1) {
            strcpy(username, messageRecu);
            strcpy(messageEnvoi, "USERNAME_OK");
            if (send(socketDialogue, messageEnvoi, LG_MESSAGE, 0) == -1) {
                perror("send");
                close(socketDialogue);
                exit(-6);
            }
        } else {
            strcpy(messageEnvoi, "Nom d'utilisateur invalide. Entrez un nom d'utilisateur valide : ");
            if (send(socketDialogue, messageEnvoi, LG_MESSAGE, 0) == -1) {
                perror("send");
                close(socketDialogue);
                exit(-6);
            }
        }
    } while (strlen(username) == 0);
    
    printf("Utilisateur '%s' connecté.\n", username);
    addClientToList(socketDialogue);

    // Envoi de l'historique des messages au nouveau client
    for (int i = 0; i < message_list->size; i++) {
        formatMessage(message_list, i, buffer);
        if (write(socketDialogue, buffer, strlen(buffer)) < 0) {
            perror("history write");
        }
    }

    // Envoi de l'annonce de connexion
    addMessageToList(message_list, username, "Connecté");
    broadcastMessages(message_list, 1, socketDialogue);

    // Boucle de communication
    while (1) {
        int messageLength = 0;
        // Receive the message length first
        if (recv(socketDialogue, &messageLength, sizeof(int), 0) <= 0) {
            perror("recv length");
            break;
        }

        memset(messageRecu, 0, LG_MESSAGE);
        // Reception du message
        if (recv(socketDialogue, messageRecu, messageLength, 0) <= 0) {
            perror("recv");
            break;
        }

        // Ajout du message à la liste
        addMessageToList(message_list, username, messageRecu);
        broadcastMessages(message_list, 1, socketDialogue);
        messageRecu[0] = '\0';
    }

    // Envoi de l'annonce de déconnexion
    addMessageToList(message_list, username, "Déconnecté");
    broadcastMessages(message_list, 1, socketDialogue);
    
    // Fermeture de la socket de dialogue
    printf("Utilisateur '%s' déconnecté.\n", username);
    removeClientFromList(socketDialogue);
    close(socketDialogue);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Déclaration des variables et des structures
    t_message_list message_list;
    message_list.size = 0;
    message_list.messages = (t_message*)malloc(sizeof(t_message) * message_list.size);
    pthread_mutex_init(&message_list.mutex, NULL);
    client_list.count = 0;
    pthread_mutex_init(&client_list.mutex, NULL);

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

    // Fixe la taille de la file d’attente au maximum
    if (listen(socketEcoute, MAX_CLIENTS) < 0) {
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