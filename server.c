serveur: 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


#define PORT 5000
#define LG_MESSAGE 256


int main(int argc, char *argv[]) {
    int socketEcoute;
    struct sockaddr_in pointDeRencontreLocal;
    socklen_t longueurAdresse;


    int socketDialogue1;
    int socketDialogue2;
    struct sockaddr_in pointDeRencontreDistant1;
    struct sockaddr_in pointDeRencontreDistant2;
    char messageRecu1[LG_MESSAGE];
    char messageRecu2[LG_MESSAGE];
    char messageEnvoi[LG_MESSAGE];
    int nb;


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


        socketDialogue1 = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant1, &longueurAdresse);
        if (socketDialogue1 < 0) {
            perror("accept");
            close(socketDialogue1);
            close(socketEcoute);
            exit(-4);
        }


        socketDialogue2 = accept(socketEcoute, (struct sockaddr *)&pointDeRencontreDistant2, &longueurAdresse);
        if (socketDialogue2 < 0) {
            perror("accept");
            close(socketDialogue1);
            close(socketDialogue2);
            close(socketEcoute);
            exit(-4);
        }
       
        if(fork() ==0){
            sprintf(messageEnvoi, "joueur1");


            // Premier message à envoyer au client 1
            switch (nb = write(socketDialogue1, messageEnvoi, LG_MESSAGE)) {
                case -1:
                    perror("Erreur en écriture 1...");
                    close(socketDialogue1);
                    close(socketDialogue2);
                    exit(-3);
                case 0:
                    fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                    return 0;
                default:
                    printf("Message %s envoyé à client 1! (%d octets)\n\n", messageEnvoi, nb);
            }


            sprintf(messageEnvoi, "joueur2");


            // Premier message à envoyer au client 2
            switch (nb = write(socketDialogue2, messageEnvoi, LG_MESSAGE)) {
                case -1:
                    perror("Erreur en écriture 2...");
                    close(socketDialogue1);
                    close(socketDialogue2);
                    exit(-3);
                case 0:
                    fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                    return 0;
                default:
                    printf("Message %s envoyé à client 2! (%d octets)\n\n", messageEnvoi, nb);
            }


            do {
                // Réception du message du client 1
                switch (nb = read(socketDialogue1, messageRecu1, LG_MESSAGE)) {
                    case -1:
                        perror("Erreur en lecture 1...");
                        close(socketDialogue1);
                        close(socketDialogue2);
                        exit(-3);
                    case 0:
                        fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                        return 0;
                    default:
                        printf("Message %s reçu par client 1! (%d octets)\n\n", messageRecu1, nb);
                }


                // Envoi du message du client 1 au client 2
                switch (nb = write(socketDialogue2, messageRecu1, LG_MESSAGE)) {
                    case -1:
                        perror("Erreur en écriture 3...");
                        close(socketDialogue1);
                        close(socketDialogue2);
                        exit(-3);
                    case 0:
                        fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                        return 0;
                    default:
                        printf("Message %s envoyé à client 2! (%d octets)\n\n", messageRecu1, nb);
                }


                if (strstr(messageRecu1, "fin") == NULL) {
                    // Réception du message du client 2
                    switch (nb = read(socketDialogue2, messageRecu2, LG_MESSAGE)) {
                        case -1:
                            perror("Erreur en lecture 2...");
                            close(socketDialogue1);
                            close(socketDialogue2);
                            exit(-3);
                        case 0:
                            fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                            return 0;
                        default:
                            printf("Message %s envoyé par client 2! (%d octets)\n\n", messageRecu2, nb);
                    }


                    // Envoi du message du client 2 au client 1
                    switch (nb = write(socketDialogue1, messageRecu2, LG_MESSAGE)) {
                        case -1:
                            perror("Erreur en écriture 4...");
                            close(socketDialogue1);
                            close(socketDialogue2);
                            exit(-3);
                        case 0:
                            fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                            return 0;
                        default:
                            printf("Message %s envoyé à client 1! (%d octets)\n\n", messageRecu2, nb);
                    }
                }
            } while (strstr(messageRecu1, "fin") == NULL);
            return 0;
        }
    }


    // Fermeture des sockets
    close(socketDialogue1);
    close(socketDialogue2);
    close(socketEcoute);
    return 0;
}

