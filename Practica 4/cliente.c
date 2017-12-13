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
int sockfd, enlace, numero_bytes;
int desarrollado=0;
char ip[20];                       //guarda la ip

struct in_addr addr;
struct sockaddr_in local, remoto;
socklen_t addr_size;


int main(int argc, char *argv[]){
    if (argc !=2 && argc !=3){
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

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
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

	if (bind(sockfd, (struct sockaddr *) &local, sizeof(local)) == -1) {
		perror("Error al hacer el bind\n");
		exit(EXIT_FAILURE);
	}

    bzero(&echoRequest, sizeof(echoRequest));
    echoRequest.ID=getpid();
    echoRequest.SeqNumber=0;
    echoRequest.icmpHeader.Checksum = 0;
    strcpy(echoRequest.payload, "payload");

    echoRequest.icmpHeader.Type=8;
    echoRequest.icmpHeader.Code=0;

    //Calculo del Checksum
    int numShorts=sizeof(echoRequest)/2;
    unsigned short int *puntero;
    unsigned int acumulado=0;
    puntero=(unsigned short int *) &echoRequest;
    int i;

    for(i=0;i<numShorts;i++){
        acumulado=acumulado+(unsigned int) *puntero;
        puntero++;
    }

    acumulado=(acumulado>>16) + (acumulado & 0x0000ffff);
    acumulado=(acumulado>>16) + (acumulado & 0x0000ffff);
    acumulado = ~acumulado;
    printf("Checksum al principio -> %d\n",(unsigned int short)acumulado);
    fflush(stdout);

    //Asignacion del cheksum
    echoRequest.icmpHeader.Checksum=(unsigned int short) acumulado;

    //Comprobacion del Checksum
    puntero=(unsigned short int *) &echoRequest;
    acumulado=0;
    for(i=0;i<numShorts;i++){
        acumulado=acumulado+(unsigned int) *puntero;
        puntero++;
    }

    acumulado=(acumulado>>16) + (acumulado & 0x0000ffff);
    acumulado=(acumulado>>16) + (acumulado & 0x0000ffff);
    acumulado = ~acumulado;
    printf("Checksum al final -> %d\n",(unsigned short int)acumulado);
    fflush(stdout);

    if((unsigned short int) acumulado != 0){
        printf("Error al calcular el checksum\n");
        exit(EXIT_FAILURE);
    }

    //Se envia el paquete ICMP
    if(sendto(sockfd,&echoRequest,sizeof(echoRequest),0,(struct sockaddr *)&remoto,sizeof(struct sockaddr_in))==-1){
        printf("Error al enviar\n");
        exit(EXIT_FAILURE);
    }

    //Se recibe la respuesta
    socklen_t addrlen= sizeof(remoto);
    numero_bytes=recvfrom(sockfd, &echoResponse, sizeof(echoResponse), 0, (struct sockaddr *) &remoto, &addrlen);
    if(numero_bytes<0){
        printf("Error al recibir\n");
        exit(EXIT_FAILURE);
    }

    switch(echoResponse.icmpHeader.Type){
        case 0:
            printf("Echo reply || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 1 ... 2:
            printf("Reserved || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 3:
            switch (echoResponse.icmpHeader.Code) {
                case 0:
                    printf("Destination network unreachable || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 1:
                    printf("Destination host unreachable || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 2:
                    printf("Destination protocol unreachable || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 3:
                    printf("Destination port unreachable || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 4:
                    printf("Fragmentation required, and DF flag set || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 5:
                    printf("Source route failed || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 6:
                    printf("Destination network unknown || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 7:
                    printf("Destination host unknown || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 8:
                    printf("Source host isolated || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 9:
                    printf("Network administratively prohibited || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 10:
                    printf("Host administratively prohibited || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 11:
                    printf("Network unreachable for ToS || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 12:
                    printf("Host unreachable for ToS || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 13:
                    printf("Communication administratively prohibited || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 14:
                    printf("Host Precedence Violation || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 15:
                    printf("Precedence cutoff in effect || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
            }
        case 4:
            printf("Source quench (congestion control) || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 5:
            switch (echoResponse.icmpHeader.Code) {
                case 0:
                    printf("Redirect Datagram for the Network || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 1:
                    printf("Redirect Datagram for the Host || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 2:
                    printf("Redirect Datagram for the ToS & network || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 3:
                    printf("Redirect Datagram for the ToS & host || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
            }
        case 6:
            printf("Alternate Host Address || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 7:
            printf("Reserved || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 8:
            printf("Echo request (used to ping) || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 9:
            printf("Router Advertisement || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 10:
            printf("Router discovery/selection/solicitation || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 11:
            switch (echoResponse.icmpHeader.Code) {
                case 0:
                    printf("TTL expired in transit || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 1:
                    printf("Fragment reassembly time exceeded || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
            }
        case 12:
            switch (echoResponse.icmpHeader.Code) {
                case 0:
                    printf("Pointer indicates the error || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 1:
                    printf("Missing a required option || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
                case 2:
                    printf("Bad length || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
                    break;
            }
        case 13:
            printf("Timestamp || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 14:
            printf("Timestamp reply || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 15:
            printf("Information Request || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 16:
            printf("Information Reply || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 17:
            printf("Address Mask Request || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 18:
            printf("Address Mask Reply || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 19:
            printf("Reserved for security || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 20 ... 29:
            printf("Reserved for robustness experiment || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 30:
            printf("Information Request || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 31:
            printf("Datagram Conversion Error || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 32:
            printf("Mobile Host Redirect || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 33:
            printf("Where-Are-You (originally meant for IPv6) || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 34:
            printf("Here-I-Am (originally meant for IPv6) || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 35:
            printf("Mobile Registration Request || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 36:
            printf("Mobile Registration Reply || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 37:
            printf("Domain Name Request || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 38:
            printf("Domain Name Reply || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 39:
            printf("SKIP Algorithm Discovery Protocol, Simple Key-Management for Internet Protocol || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 40:
            printf("Photuris, Security failures || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 41:
            printf("ICMP for experimental mobility protocols such as Seamoby [RFC4065] || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 42 ... 252:
            printf("Reserved || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 253:
            printf("RFC3692-style Experiment 1 || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 254:
            printf("RFC3692-style Experiment 2 || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;
        case 255:
            printf("Reserved || Type %d || Code %d\n",echoResponse.icmpHeader.Type, echoResponse.icmpHeader.Code);
            break;

    }
//**********************************************************************************
    return(EXIT_SUCCESS);
}
