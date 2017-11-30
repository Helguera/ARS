//------------------------------------------------------------------------------------------------------------------
//*********************************** PRACTICA TEMA 7, HELGUERA LOPEZ JAVIER **************************************
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

#define MAXBUFFSIZE 1024
#define MAXTAMBYTE 512

//definicion de las variables del programa
int puerto, sockfd, numero_bytes, operacion, datagrama_size, codigo, num_bloque, num_bloque_rec, bytes_envio, envio_size;
int desarrollado=0;
int longitud=512;
int primer_datagrama=1;            
int salir=0;                       //variable para salir del programa cuando se hayan recibido todos los bytes
char ip[20];                       //guarda la ip del servidor
char buffer[MAXBUFFSIZE];          //guarda la respuesta recibida del servidor
char nombre_fichero[100];          //guarda el nombre del fichero
char datagrama[MAXBUFFSIZE];
unsigned char *bloque_envio;
struct servent *serv;              //se utilizara para obtener el puerto del servicio tftp
struct sockaddr_in direccion;
struct in_addr addr;
socklen_t addr_size;

FILE *fichero;

//---------------------------------------------------------------------------------------
int main(int argc, char *argv[]){
    //Se realiza la comprobacion de los datos introducidos como parametros al programa
    if (argc != 5 && argc!=4){           //El numero de parametros permitidos es 1 o 2
        fprintf(stderr,"uso: %s ip-servidor {-r|-w} archivo [-v]\n", argv[0]);        
        exit(EXIT_FAILURE);
    }else{
        if (inet_aton(argv[1], &addr) == 0) {
            //Si el formato de la direccion ip no es valido
            fprintf(stderr, "Direccion no valida\n");
            exit(EXIT_FAILURE);
        }

        strcpy(ip, argv[1]);
        serv = getservbyname("tftp", "udp");
        if (serv == NULL) {
            fprintf(stderr, "No se ha encontrado el servicio 'tftp' mediante el protocolo 'udp");
            exit(EXIT_FAILURE);
        }
        puerto = serv->s_port;

        //Selecciona la operacion de lectura o de escritura ("-r" o "-w")
        if(strcmp(argv[2],"-r")==0 || strcmp(argv[2],"-w")==0){
            if(strcmp(argv[2],"-r")==0){
                operacion=1;        //Lectura
            }
            if(strcmp(argv[2],"-w")==0){
                operacion=2;        //Escritura
            }
        }else{
            fprintf(stderr,"uso: %s ip-servidor {-r|-w} archivo [-v]\n", argv[0]);        
            exit(EXIT_FAILURE);
        }

        //Se guarda el nombre del fichero
        strcpy(nombre_fichero, argv[3]);

        //Selecciona el modo desarrollado si el usuario ha escrito "-v"
        if(argc==5){
            if(strcmp(argv[4],"-v")==0){
                desarrollado=1;
            }else{
                fprintf(stderr,"uso: %s ip-servidor {-r|-w} archivo [-v]\n", argv[0]);        
                exit(EXIT_FAILURE);
            }
        }
     }

    //Se inicia el sockett UDP (sock_DGRAM)
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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
    
    if(operacion == 1){     //Modo lectura (-r)
        while (salir==0){
            if (primer_datagrama==1){
                //Fichero donde se guardara la informacion recibida
                fichero=fopen(nombre_fichero,"w");
                if(fichero==NULL){
                    printf("No se ha podido crear el fichero\n");
                    exit(EXIT_FAILURE);
                }

                //Se crea el datagrama RRQ
                datagrama[0]=0;
                datagrama[1]=1;
                strcpy(datagrama+2, nombre_fichero);
                strcpy(datagrama+(strlen(nombre_fichero)+3),"octet");
            }else{
                num_bloque_rec=(unsigned char)datagrama[3]+((unsigned char)datagrama[2]<<8);
                if(num_bloque_rec!=num_bloque){
					printf("Error, se recibio el bloque %d pero se esperaba el bloque %d\n",num_bloque_rec,num_bloque);
					exit(EXIT_FAILURE);
                }
                //Se cambia el codigo para que sea un datagrama de datos
                datagrama[0]=0;
                datagrama[1]=4;

            }

            datagrama_size=2+strlen(nombre_fichero)+1+strlen("octet")+1;
            if(sendto(sockfd,datagrama,sizeof(char)*datagrama_size,0,(struct sockaddr*) &direccion,sizeof(direccion))==-1){
                printf("Error al enviar\n");
                exit(EXIT_FAILURE);
            }
            if (primer_datagrama==1 && desarrollado==1){
                printf("Enviada solicitud de %s a servidor tftp en %s\n",nombre_fichero,ip);
            }
            if(primer_datagrama==0 && desarrollado==1){
                printf("Enviamos el ACK del bloque %d\n",num_bloque);
            }
            

            socklen_t addrlen= sizeof(direccion);
            numero_bytes = recvfrom(sockfd,datagrama,MAXBUFFSIZE,0,(struct sockaddr*)&direccion,&addrlen);
            num_bloque = num_bloque+1;
            if(desarrollado==1){
                printf("Recibido bloque del servidor tftp\n");
                if (primer_datagrama==1){
                    printf("Es el primer bloque (numero de bloque 1)\n");
                }else{
                    printf("Es el bloque con codigo %d\n",num_bloque);
                }
            }

            codigo=datagrama[1];
            if(codigo == 5){    //Mensaje de error
                printf("Error %d (%s)\n\n",datagrama[3],&datagrama[4]);
                exit(EXIT_FAILURE);
            }

            fwrite(datagrama+4, sizeof(char),numero_bytes-4,fichero);
            if(numero_bytes<MAXTAMBYTE){
                printf("El bloque %d era el ultimo: cerramos el fichero\n",num_bloque);
                fflush(stdout);
                fclose(fichero);
                salir=1;
            }
            primer_datagrama=0;
        }


    }else{      //Modo escritura (-w)
        while(salir==0){
            if(primer_datagrama==1){
                fichero=fopen(nombre_fichero,"r");
                if (fichero==NULL){
                    printf("No existe el fichero\n");
                    exit(EXIT_FAILURE);
                }
    
                num_bloque=0;
            
                //Se crea el datagrama WRQ
                datagrama[0]=0;
                datagrama[1]=2;
                strcpy(datagrama+2, nombre_fichero);
                strcpy(datagrama+(strlen(nombre_fichero))+3,"octet");
                datagrama_size=2+strlen(nombre_fichero)+1+strlen("octect")+1;
            }else{
                if(!feof(fichero)){
                    bytes_envio=fread(datagrama+4,sizeof(char),512,fichero);
                }else{
                    bytes_envio=0;
                }
    
                //datagrama[0]=0;
                datagrama[1]=3;
    
                bloque_envio=(unsigned char*) &num_bloque;
                datagrama[2]=bloque_envio[1];
                datagrama[3]=bloque_envio[0];
    
                datagrama_size=2+2+bytes_envio;
    
                num_bloque_rec=(unsigned char) datagrama[3] + ((unsigned char)datagrama[2]<<8);
                if(num_bloque_rec!=num_bloque){
                    printf("Error, se recibio el bloque %d pero se esperaba el bloque %d\n",num_bloque_rec,num_bloque);
                    exit(EXIT_FAILURE);
                }
            }
    
            socklen_t addrlen= sizeof(direccion);
            if((bytes_envio=sendto(sockfd,datagrama,sizeof(char)*datagrama_size,0,(struct sockaddr *) &direccion,addrlen))<0){
                printf("Error al enviar\n");
                exit(EXIT_FAILURE);
            }

            if(primer_datagrama==1 && desarrollado==1){
                printf("Enviada solicitud de escritura de %s a servidor tftp en %s\n",nombre_fichero,ip);
            }
            if(primer_datagrama==0 && desarrollado==1){
                printf("Enviamos el datagrama del bloque %d\n",num_bloque);
            }
    
            if((num_bloque_rec = recvfrom(sockfd,datagrama,MAXTAMBYTE,0,(struct sockaddr *)&direccion,&addrlen))<0){
                printf("Error al recibir\n");
                exit(EXIT_FAILURE);
            }else{
                if(desarrollado==1){
                    printf("Recibido el ACK\n");
                }
            }
    
            codigo=datagrama[1];
            if(codigo==5){
                printf("Error %d (%s)\n\n",datagrama[3],&datagrama[4]);
                exit(EXIT_FAILURE);
            }
    
            if(primer_datagrama==0 && bytes_envio<MAXTAMBYTE){
                fclose(fichero);
                printf("El bloque %d era el ultimo: cerramos el fichero\n",num_bloque);
                salir=1;
            }

            num_bloque+=1;
            primer_datagrama=0;
        }
    }
    return(EXIT_SUCCESS);
}