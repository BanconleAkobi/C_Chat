

#include "socketutil.c"

int main(){

        char *ip = "142.250.201.174"; 

    int socketID = createTCPIpv4Socket(); 
    struct sockaddr_in *address = createIpv4Address("127.0.0.1", 2000); 


    /*
        connecter
        connect(l'id du socket, l'adresse du server, la longueur de cette adresse)
        attention à caster pour eviter les erreurs: 
    */
   
   int result =  connect(socketID, (struct sockaddr*)address, sizeof(*address));

    if(result == 0){
        printf("good connexion \n");
    }


    char* message;

    message = "GET \\ HTTP/1.1\r\nHost:google.com\r\n\r\n" ;

    send(socketID, message, strlen(message), 0); 

    char buffer[1024] ;

    recv(socketID, buffer, 1024, 0); 
    printf("La réponse est: %s\n", buffer); 

}
