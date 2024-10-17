#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(){

    /*
        *creer un socket: 
        *socket(l'adresse famille, le type socket(udp ou tcp), 0 ) => retourne un entier
        * > 0 si ça s'est bien passé
        *
    */
    int soketID = socket(AF_INET, SOCK_STREAM, 0); 


    char *ip = "142.250.201.174"; 
    struct sockaddr_in address; 
    // addresse du serveur de type sockaddr_in et definir les proprietes
    //port du server qui ecoute
    //connvertir le nombre du port et prend met les byte dans le bon sens
    address.sin_port = htons(80); 
    address.sin_family = AF_INET; 
    //inet_pton lui dit tu convertis ip en ipv4 pour l'adapter et le met dans address.sin_addr.s_addr 
    inet_pton(AF_INET, ip, &address.sin_addr.s_addr); 

    /*
        connecter
        connect(l'id du socket, l'adresse du server, la longueur de cette adresse)
        attention à caster pour eviter les erreurs: 
    */
   int result =  connect(socketID, &address, sizeof(address));

    if(result == 0){
        printf("good connexion");
    }

}