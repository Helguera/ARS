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

//Cmabiar el nombre al socket enlace
int sockfd, enlace;
int desarrollado=0;
char ip[20];                       //guarda la ip

struct in_addr addr;
struct sockaddr_in local, remoto;
socklen_t addr_size;


int main(int argc, char *argv[]){

    if (argc !=2 || argc !=3){
        fprintf(stderr,"uso: %s ip-servidor [-v]\n", argv[0]);
        exit(EXIT_FAILURE);
    }else{
        if(inet_aton(argv[1], &addr)==0){
            fprintf(stderr, "Direccion ip no valida\n");
            exit(EXIT_FAILURE);
        }
        strcpy(ip, argv[1]);

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

    remoto.sin_family=AF_INET;
    remoto.sin_port=0;
    remoto.sin_addr.s_addr = inet_addr(ip);
    //memset(remoto.sin_zero, '\0', sizeof remoto.sin_zero);
    //addr_size = sizeof remoto;

    local.sin_family=AF_INET;
    local.sin_port=0;
    local.sin_addr.s_addr =INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &local, sizeof(struct sockaddr_in)) == -1) {
		perror("Error al hacer el bind\n");
		exit(EXIT_FAILURE);
	}

    bzero(&echoRequest, sizeof(echoRequest));
    echoRequest.ID=getpid();
    echoRequest.SeqNumber=0;
    strcpy(echoRequest.payload, "payload");

    echoRequest.icmpHeader.Type=8;
    echoRequest.icmpHeader.Code=0;

    //Calculo del Checksum
    int numShorts=sizeof(echoRequest)/2;
    unsigned short int *puntero;
    int acumulado=0;
    puntero=(unsigned short int *) &echoRequest;


























//**********************************************************************************
    return(EXIT_SUCCESS);
}
