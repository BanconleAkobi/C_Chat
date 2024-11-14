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
    int nb;
    char lettresDonnees[27] = "";
    int joueur;
    int nbEssaisRates = 0;
    int nbEssaisRatesMax = 10;
    char mot[LG_MESSAGE];
    char motAffiche[LG_MESSAGE] = "";


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


    // Reception du message de début de partie
    switch (nb = read(descripteurSocket, messageRecu, LG_MESSAGE)) {
        case -1:
            perror("Erreur en lecture...");
            close(descripteurSocket);
            exit(-3);
        case 0:
            fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
            return 0;
        default:
            printf("%s\n\n", messageRecu);
    }


    if (strcmp(messageRecu, "joueur1") == 0) {
        joueur = 1;
        printf("Ecrivez un mot à faire deviner :\n");
        scanf("%s", mot);


        for (int i = 0; i < strlen(mot); i++) {
            if (mot[i] >= 'a' && mot[i] <= 'z') {
                mot[i] = mot[i] - 32;
            }
            motAffiche[i] = '-';
        }


        sprintf(messageEnvoi, "start %ld", strlen(mot));


        // Envoie du premier message
        switch (nb = write(descripteurSocket, messageEnvoi, LG_MESSAGE)) {
            case -1:
                perror("Erreur en écriture...");
                close(descripteurSocket);
                exit(-3);
            case 0:
                fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                return 0;
            default:
                printf("%s\n\n", messageEnvoi);
        }
    } else {
        joueur = 2;
    }


    // Boucle de jeu
    while ((joueur == 1 && (nbEssaisRates < nbEssaisRatesMax && strchr(motAffiche, '-') != NULL)) || joueur == 2) {
        // Reception du message de début de partie
        switch (nb = read(descripteurSocket, messageRecu, LG_MESSAGE)) {
            case -1:
                perror("Erreur en lecture...");
                close(descripteurSocket);
                exit(-3);
            case 0:
                fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                return 0;
            default:
                if (joueur == 2) {
                    printf("%s\n\n", messageRecu);
                    if (strstr(messageRecu, "fin") != NULL) {
                        close(descripteurSocket);
                        return 0;
                    }
                }
        }


        if (joueur == 1) {
            // Si la lettre a déjà été donnée ou est invalide
            if (strchr(lettresDonnees, messageRecu[0]) != NULL || toupper(messageEnvoi[0]) < 'A' || toupper(messageEnvoi[0]) > 'Z') {
                sprintf(messageEnvoi, "La lettre déjà utilisée ou invalide.\nMot : %s\nEntrez une nouvelle lettre :", motAffiche);
            } else if (strchr(mot, messageRecu[0]) != NULL) {
                strcat(lettresDonnees, messageRecu);


                for (int i = 0; i < strlen(mot); i++) {
                    if (strchr(lettresDonnees, mot[i]) != NULL) {
                        motAffiche[i] = mot[i];
                    } else {
                        motAffiche[i] = '-';
                    }
                    if (strchr(motAffiche, '-') != NULL) {
                        sprintf(messageEnvoi, "La lettre est dans le mot.\nMot : %s\nEntrez une nouvelle lettre :", motAffiche);
                    } else {
                        sprintf(messageEnvoi, "Vous avez trouvé le mot entier.\nMot : %s\nfin", motAffiche);
                    }
                }
            } else {
                strcat(lettresDonnees, messageRecu);


                nbEssaisRates++;
                if (nbEssaisRates < nbEssaisRatesMax) {
                    sprintf(messageEnvoi, "La lettre n'est pas dans le mot. %d essais restants.\nMot : %s\nEntrez une nouvelle lettre :", nbEssaisRatesMax - nbEssaisRates, motAffiche);
                } else {
                    sprintf(messageEnvoi, "Partie perdue, le mot était : '%s'\nfin", mot);
                }
            }
        } else {
            scanf(" %c", messageEnvoi);
            messageEnvoi[0] = toupper(messageEnvoi[0]);
        }


        // Envoie du premier message
        switch (nb = write(descripteurSocket, messageEnvoi, LG_MESSAGE)) {
            case -1:
                perror("Erreur en écriture...");
                close(descripteurSocket);
                exit(-3);
            case 0:
                fprintf(stderr, "La socket a été fermée par le serveur !\n\n");
                return 0;
            default:
                break;
        }
    }


    // Fermeture du socket
    close(descripteurSocket);
    return 0;
}

























