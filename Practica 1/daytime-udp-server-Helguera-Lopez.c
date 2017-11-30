//------------------------------------------------------------------------------------------------------------------
//*********************************** PRACTICA TEMA 5, HELGUERA LOPEZ JAVIER **************************************
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

#define MAXBUFFSIZE 4096    //Maximo tamaño de los buffers del programa

void extrae_fecha(void);    //definicion de la funcion que extrae la fecha del sistema a una variable

//Definicion de las variables del programa
int puerto, udpSocket, numero_bytes; 
char recibido[MAXBUFFSIZE];         //buffer donde se guarda el mensaje recibido por el cliente
char buff[MAXBUFFSIZE];             //buffer donde se guarda la fecha y hora del sistema
char hostname[MAXBUFFSIZE];         //buffer donde se guardara el hostname y el mensaje final
char temporal[MAXBUFFSIZE];         //buffer temporal
char comprobacion[5];
struct servent *serv;               //se utilizara si no se indica el puerto en la ejecucion del programa

struct sockaddr_in dir_servidor;
struct sockaddr_storage serverStorage;
socklen_t addr_size, client_addr_size;

FILE *fichero;                         //fichero auxiliar para obtener la fecha y hora del sistema

//------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[]){
    //Se realiza la comprobacion de los datos introducidos como parametros al programa
    if (argc !=1 && argc !=3){          //El numero permitido de parametros es 0 o 1
        fprintf(stderr, "Uso: %s <-p puerto>\n", argv[0]);
        exit(EXIT_FAILURE);
    }else{
        if(argc == 1){
            //Si no se ha introducido el puerto, el programa lo busca en /etc/services
            serv = getservbyname("daytime", "udp");
            if(serv == NULL){
                fprintf(stderr, "No se ha encontrado el servicio 'daytime' mediante el protocolo 'udp'");
                exit(EXIT_FAILURE);
            }
            puerto = serv->s_port;
        }
        if(argc == 3){
            //Se utiliza el puerto introducido por el usuario
            strcpy(comprobacion,"-p");
            if(strcmp(argv[1],comprobacion)==0){
                puerto = htons(atoi(argv[2]));
            }else{
                fprintf(stderr, "Uso: %s <-p puerto>\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            
        }
    }

    gethostname(hostname,MAXBUFFSIZE);          //Se obtiene el nombre de la maquina que ejecuta el servidor

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);         //Se inicia el socket UDP

    //Configuraciones de la direccion del servidor
    dir_servidor.sin_family = AF_INET;          //familia = internet
    dir_servidor.sin_port = puerto;             //Se le asigna el puerto
    dir_servidor.sin_addr.s_addr = inet_addr("127.0.0.1");      //Se le asigna la ip, en este caso la local
    memset(dir_servidor.sin_zero, '\0', sizeof dir_servidor.sin_zero);      

    bind(udpSocket, (struct sockaddr *) &dir_servidor, sizeof(dir_servidor));

    addr_size = sizeof serverStorage;

    //Comienza el bucle a la espera de alguna solicitud
    while(1){
        //Una vez se recibe una solicitud, se guarda el puerto y la ip del cliente para poder responderle
        numero_bytes = recvfrom(udpSocket,recibido,MAXBUFFSIZE,0,(struct sockaddr *)&serverStorage, &addr_size);
        extrae_fecha();
        strcpy(temporal, hostname);
        strcat(temporal, ": ");
        strcat(temporal, buff);
        sendto(udpSocket,temporal,numero_bytes,0,(struct sockaddr *)&serverStorage,addr_size);      //Envia al cliente el host, la fecha y la hora del sistema
    }
    exit(EXIT_SUCCESS);
}
//-------------------------------------------------------------------------------------------------------


//Funcion que extrae la fecha y hora del sistema y la guarda en la variable buff
void extrae_fecha(void){
    system("date> /tmp/tt.txt");
    fichero = fopen("/tmp/tt.txt","r");
    if(fgets(buff,MAXBUFFSIZE,fichero)==NULL){
        fprintf(stderr, "Error en system(), en fopen(), o en fgets()\n");
        exit(EXIT_FAILURE);
    }
    return;
}
