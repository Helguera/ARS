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
#include <signal.h>

#define MAXBUFFSIZE 4096    //Maximo tamaño de los buffers del programa

void signal_handler(int);   //definicion de la funcion que cierra los sockets antes de cerrar el servidor
void extrae_fecha(char*);    //definicion de la funcion que extrae la fecha del sistema a una variable

//Definicion de las variables del programa
int puerto, tcpSocket, newtcpSocket, numero_bytes, pid; 

char hostname[MAXBUFFSIZE];         //buffer donde se guardara el hostname y el mensaje final
char comprobacion[5];
struct servent *serv;               //se utilizara si no se indica el puerto en la ejecucion del programa


struct sockaddr_in dir_servidor;
struct sockaddr_in dir_cliente;
socklen_t longitud_dir_cliente;

FILE *fichero;                         //fichero auxiliar para obtener la fecha y hora del sistema

//------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[]){
    signal(SIGINT, signal_handler);
    //Se realiza la comprobacion de los datos introducidos como parametros al programa
    if (argc !=1 && argc !=3){          //El numero permitido de parametros es 0 o 1
        fprintf(stderr, "Uso: %s <-p puerto>\n", argv[0]);
        exit(EXIT_FAILURE);
    }else{
        if(argc == 1){
            //Si no se ha introducido el puerto, el programa lo busca en /etc/services
            serv = getservbyname("daytime", "tcp");
            if(serv == NULL){
                fprintf(stderr, "No se ha encontrado el servicio 'daytime' mediante el protocolo 'tcp'");
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

    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);         //Se inicia el socket TCP

    //Configuraciones de la direccion del servidor
    dir_servidor.sin_family = AF_INET;          //familia = internet
    dir_servidor.sin_port = puerto;             //Se le asigna el puerto
    dir_servidor.sin_addr.s_addr = inet_addr("127.0.0.1");      //Se le asigna la ip, en este caso la local
    memset(dir_servidor.sin_zero, '\0', sizeof dir_servidor.sin_zero);      

    //bindea el socket con la direccion del servidor y el puerto
    bind(tcpSocket,(struct sockaddr *)&dir_servidor, sizeof(dir_servidor));
 
    //escucha conexiones entrantes de clientes (1024 conexiones simultaneas como maximo, puede ser cualquier numero)
    listen(tcpSocket,1024);
 
    while(1){           //el proceso padre esperando conexiones
        longitud_dir_cliente=sizeof(dir_cliente);
        newtcpSocket=accept(tcpSocket,(struct sockaddr*)&dir_cliente,&longitud_dir_cliente);
        //printf("Cliente: %s\n",inet_ntoa(dir_cliente.sin_addr));
        pid=fork();
        if(pid==0){      
            while(1){
                char temporal[MAXBUFFSIZE];         //buffer temporal
                char buff[MAXBUFFSIZE];             //buffer donde se guarda la fecha y hora del sistema
                char pid[MAXBUFFSIZE];              //debugger 
		        char ppid[MAXBUFFSIZE];             //debugger
		
		        sprintf(pid, "%d", getpid());       //debugger
		        sprintf(ppid, "%d", getppid());     //debugger

		        extrae_fecha(buff);
                strcpy(temporal, hostname);
                strcat(temporal, ": ");
                strcat(temporal, buff);
               	//strcat(temporal, "Hijo: ");       //debugger
		        //strcat(temporal, pid);            //debugger
		        //strcat(temporal, "    Padre: ");  //debugger
		        //strcat(temporal, ppid);           //debugger
                send(newtcpSocket, temporal ,MAXBUFFSIZE,0);      //se envia el mensaje el cliente
            }
            exit(0);
        }
        else{
            close(newtcpSocket);
        }
    }
 

    exit(EXIT_SUCCESS);
}
//-------------------------------------------------------------------------------------------------------


//Funcion que extrae la fecha y hora del sistema y la guarda en la variable buff
void extrae_fecha(char *cadena){
    system("date> /tmp/tt.txt");
    fichero = fopen("/tmp/tt.txt","r");
    if(fgets(cadena,MAXBUFFSIZE,fichero)==NULL){
        fprintf(stderr, "Error en system(), en fopen(), o en fgets()\n");
        exit(EXIT_FAILURE);
    }
    return;
}

//Funcion que cierra los sockets antes de cerrar el servidor
void signal_handler(int sig){
    signal(sig, SIG_IGN);
    shutdown(tcpSocket, SHUT_RDWR);
    shutdown(newtcpSocket, SHUT_RDWR);
    printf("\n***Los sockets han sido cerrados***\n");
    fflush(stdout);
    exit(EXIT_SUCCESS);
}
