//------------------------------------------------------------------------------------------------------------------
//*********************************** PRACTICA TEMA 8, HELGUERA LOPEZ JAVIER **************************************
//------------------------------------------------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "ip-icmp-ping.h"

int sockfd;
int desarrollado=0;

struct in_addr addr;


int main(int argc, char *argv[]){

    if (argc !=2 || argc !=3){
        fprintf(stderr,"uso: %s ip-servidor [-v]\n", argv[0]);
        exit(EXIT_FAILURE);
    }else{
        if(inet_aton(argv[1], &addr)==0){
            fprintf(stderr, "Direccion ip no valida\n");
            exit(EXIT_FAILURE);
        }
        if(argc==3){
            if(strcmp(argv[2],"-v")==0){
                desarrollado=1;
            }else{
                fprintf(stderr,"uso: %s ip-servidor [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

//**********************************************************************************

    ECHORequest echoRequest;
    ECHOResponse echoResponse;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1){
        fprintf(stderr, "Error al crear el socket");
        exit(EXIT_FAILURE);
    }



    return(EXIT_SUCCESS);
}
