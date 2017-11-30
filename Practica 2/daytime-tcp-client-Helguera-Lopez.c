//------------------------------------------------------------------------------------------------------------------
//*********************************** PRACTICA TEMA 6, HELGUERA LOPEZ JAVIER **************************************
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

 #define MAXBUFFSIZE 1024           //maximo tamaño de los buffers del programa
 
 //definicion de las variables del programa
 int puerto, sockfd, numero_bytes;
 char ip[20];                       //guarda la ip del servidor
 char buffer[MAXBUFFSIZE];          //guarda la respuesta recibida del servidor
 char comprobacion[5];
 struct servent *serv;              //se utilizara si no se indica el puerto del servidor en la ejecucion del programa
 struct sockaddr_in direccion;
 struct in_addr addr;
 socklen_t addr_size;
 
 //-------------------------------------------------------------------------------------------------------------------
 int main(int argc, char *argv[]){
     //Se realiza la comprobacion de los datos introducidos como parametros al programa
     if (argc != 2 && argc != 4){           //El numero de parametros permitidos es 1 o 2
        fprintf(stderr,"uso: %s <ipservidor> <-p puerto>\n", argv[0]);        
        exit(EXIT_FAILURE);
     }else{
        if (inet_aton(argv[1], &addr) == 0) {
            //Si el formato de la direccion ip no es valido
            fprintf(stderr, "Direccion no valida\n");
            exit(EXIT_FAILURE);
        }
        if (argc == 2) {
            //Si el usuario no introduce el puerto
            strcpy(ip, argv[1]);
            serv = getservbyname("daytime", "tcp");
            if (serv == NULL) {
                fprintf(stderr, "No se ha encontrado el servicio 'daytime' mediante el protocolo 'tcp'");
                exit(EXIT_FAILURE);
            }
            puerto = serv->s_port;
        }
   
        if (argc == 4){
            //Se utiliza el puerto introducido por el usuario
            strcpy(ip, argv[1]);
            strcpy(comprobacion,"-p");
            if(strcmp(argv[2],comprobacion)==0){
                puerto = htons(atoi(argv[3]));
            }else{
                fprintf(stderr,"uso: %s <ipservidor> <-p puerto>\n", argv[0]);        
                exit(EXIT_FAILURE);
            }           
        }
     }

     //Se inicia el socket TCP (sock_STREAM)
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd == -1) {
        fprintf(stderr, "Error al crear el socket");
        exit(EXIT_FAILURE);
     }

     //Configuraciones de la direccion del servidor
     direccion.sin_family=AF_INET;                  //familia = internet
     direccion.sin_port=puerto;                     //Se le asigna el puerto
     direccion.sin_addr.s_addr = inet_addr(ip);     //Se le asigna la ip, introducida por el usuario
     memset(direccion.sin_zero, '\0', sizeof direccion.sin_zero);  

     addr_size = sizeof direccion;
     connect(sockfd, (struct sockaddr *) &direccion, addr_size);        //Se conecta al servidor

     recv(sockfd, buffer, 1024, 0);                 //Se recibe el mensaje, no hay que enviar nada previamente
     printf("%s\n",buffer); 

     exit(EXIT_SUCCESS);
 }

 //-------------------------------------------------------------------------------------------------------------------------
