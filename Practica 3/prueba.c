

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>


#define ERROR -1
#define TRUE 1
#define FALSE 0
#define SIZE_BUFF 516


int main(int argc, char *argv[])
{



	int modo_v,fin;
	int tam_envio; // tamaño e envio
	unsigned char *cad_bloque;
	struct servent *servicio;
	uint16_t puerto;
	int opcode; //codigo de operacion
	int errcode; //codigo de error
	char errmensaje[512]; // mensaje de error
	int primer_datagrama=TRUE; //control de si es el primer datagrama o no
	int bloque= 0; // numero de bloque
	int bloque_recibido; // bloque que es devuelto por el servidor
	struct sockaddr_in local, remoto;
	int bytes;
	char datagrama[SIZE_BUFF];
	int tam_datagrama;

	int num_socket;
	FILE *fich;


	//comprobamos los argumentos

	if(argc ==5)
	{
		if(strcmp(argv[4], "-v")==0)
		{
			modo_v=TRUE;
		}
		else
		{
			modo_v=FALSE;
			printf("Sintaxis: ./tftp-client ip-servidor {-r|-w} archivo [-v]\n");
			printf("Se ignora la ultima opcion\n");
		}

	}
	else if(argc==4)
	{
		modo_v=FALSE;
	}

	else
	{

		printf("Sintaxis: ./tftp-client ip-servidor {-r|-w} archivo [-v]\n");
		exit(ERROR);
	}


	//Creamos el socket UDP

	num_socket= socket(AF_INET, SOCK_DGRAM,0);
	if(num_socket ==-1)
	{
		printf("Error en la creacion del socket\n");
		exit(ERROR);
	}

	//Enlazamos el socket con la maquina local

	local.sin_family = AF_INET;
	local.sin_addr.s_addr = INADDR_ANY;
	local.sin_port = 0;

	if(bind(num_socket, (struct sockaddr *) &local, sizeof(struct sockaddr_in))<0)
	{
		printf("Error en el enlace de la maquina local al socket\n");
		perror("ERROR: tftp");
		exit(ERROR);
	}

	//obtenemos la ip en network byte order y seguimos introduciendo mas datos de la maquina remota

	inet_aton(argv[1],&remoto.sin_addr);
	servicio = getservbyname("tftp","udp");
	if(!servicio)
	{
		printf("Error, no se ha obtenido el servicio\n");
		exit(ERROR);
	}
	else
	remoto.sin_port = servicio -> s_port;
	remoto.sin_family = AF_INET;

	fin=FALSE;

	//comprobamos el modo si es de lectura o de escritura y procedemos a ello

	if((strcmp(argv[2],"-r")==0))
	{
	// MODO LECTURA

		while(fin== FALSE)
		{
			if(primer_datagrama==TRUE) //Tratamos el primer datagrama
			{
				if((modo_v)==TRUE)
				{
					printf("Enviada solicitud de lectura de fichero %s a servidor tftp en %s \n", argv[3],argv[1]);
				}
				bloque =0;

				//Creamos el datagrama RRQ

				datagrama[0]=0;
				datagrama[1]=1;
				strcpy(datagrama+2,argv[3]);
				strcpy(datagrama+(strlen(argv[3])+3),"octet");

				//Creamos el fichero donde se almacenaran los bloques que leamos

				fich=fopen(argv[3],"w");
				if(fich==NULL)
				{
					printf("Error en la creacion del fichero en modo escritura\n");
					exit(ERROR);
				}
			}

			else // No es el primer datagrama y tendremos que generar el datagrama ACK
			{
				if(modo_v==TRUE)
				{
					if(bloque==1)
					{
						printf("Es el primer bloque (numero de bloque 1) \n");
					}
					else
					{
						printf("Es el bloque con codigo %d.\n",bloque);
					}
					printf("Enviamos el ACK del bloque %d. \n",bloque);

				}
				bloque_recibido=(unsigned char)datagrama[3]+((unsigned char)datagrama[2]<<8);
				if(bloque_recibido!=bloque)
				{
					printf("Numero de bloque recibido erroneo, no se corresponde con el esperado, se esperaba %d y se recibio %dv \n",bloque, bloque_recibido);
					exit(ERROR);
				}

				//preparamos el datagrama ACK para enviar

				datagrama[0]=0;
				datagrama[1]=4;
			}


			tam_datagrama=2+strlen(argv[3])+1+strlen("octet")+1; // el tama�o del datagrama (sea ACK o no) se compone de esos bytes
			//enviamos una peticion o respuesta al servidor

			if(sendto(num_socket,datagrama,sizeof(char)*tam_datagrama,0,(struct sockaddr*) &remoto,sizeof(remoto))<0)
			{
				printf("Error en sendto\n");
				exit(ERROR);
			}

			socklen_t addrlen= sizeof(remoto);
			bytes = recvfrom(num_socket,datagrama,SIZE_BUFF,0,(struct sockaddr *)&remoto,&addrlen);
			if(bytes < 0)
			{
				printf("Error en recvfrom \n");
				exit(ERROR);
			}

			bloque++;
			if(modo_v == TRUE)
				printf("Recibido bloque del servidor tftp\n");

			//comprobamos si es un mensaje de error
			opcode=datagrama[1];
			if(opcode==5) // si lo que hemos recibido tiene un codigo de operacion igual a 5 es que no son datos y miramos el mensaje que nos devuelve
			{
				errcode=datagrama[3];
				strcat(errmensaje,datagrama+4);
				printf("Errcode %d: %s\n",errcode,errmensaje);
				fclose(fich);
				remove(argv[3]);
				exit(ERROR);

			}

			fwrite(datagrama+4,sizeof(char),bytes-4,fich);
			if(bytes<516)// Si es el ultimo bloque cerramos el fichero y salimos del bucle
			{
				if(modo_v==TRUE)
					printf("el bloque %d era el ultimo: cerramos el fichero.\n",bloque);
				//Cerramos el fichero
				if(fclose(fich)==EOF)
				{
					printf("Error en fclose\n");
					exit(ERROR);
				}
				fin=TRUE;
			}
			primer_datagrama=FALSE;



		}
	}
	else if(strcmp(argv[2],"-w")==0)
	{
	// MODO ESCRITURA

		while(fin == FALSE)
		{
			if(primer_datagrama==TRUE) // Tratamos el primer datagrama
			{
				if(modo_v==TRUE)
					printf("Enviada solicitud de escritura de fichero %s a servidor tftp en %s\n",argv[3],argv[1]);

				//abrimos el fichero local para la lectura
				fich = fopen(argv[3],"r");
				if(fich == NULL)
				{
					printf("Error de apertura del fichero para lectura\n");
					exit(ERROR);
				}
				bloque =0; //comenzamos con el bloque 0

				//Creamos el datagrama WRQ

				datagrama[0]=0;
				datagrama[1]=2;
				strcpy(datagrama+2,argv[3]);
				strcpy(datagrama+(strlen(argv[3]))+3,"octet");
				tam_datagrama=2+strlen(argv[3])+1+strlen("octet")+1;
			}
			else
			{
				if(modo_v==TRUE)
				{
					if(bloque==1)
						printf("Es el primer bloque (numero de bloque 1). \n");
					else
					{
						printf("Es el bloque con codigo %d.\n",bloque);
					}
					printf("Enviamos el datagrama del bloque %d.\n", bloque);
				}

				//obtenemos el tama�o de datos para enviar

				if(!feof(fich))
				{
					tam_envio=fread(datagrama+4,sizeof(char),512,fich);
				}
				else
				{
					tam_envio=0;
				}

				datagrama[1]=3;

				//hay que incluir el bloque en el datagrama para enviar

				cad_bloque= (unsigned char*) &bloque;
				datagrama[2]=cad_bloque[1];
				datagrama[3]=cad_bloque[0];

				tam_datagrama = 2+2+ tam_envio; //los bytes de la opcode+bytes del bloque mas los datos

				//Comprobamos que el bloque recibido es el correcto
				bloque_recibido= (unsigned char) datagrama[3] + ((unsigned char)datagrama[2]<<8);
				if(bloque_recibido!= bloque)
				{
					printf("Numero de bloque recibido erroneo, no se corresponde con el esperado, se esperaba %d y se recibio %dv \n",bloque, bloque_recibido);
					exit(ERROR);
				}

			}

			//Enviamos el datagrama con los datos leidosi
			tam_envio=sendto(num_socket,datagrama,sizeof(char)*tam_datagrama,0,(struct sockaddr *) &remoto, sizeof(remoto));
			if(tam_envio<0)
			{
				printf("Error en sendto\n");
				exit(ERROR);
			}

			//recibimos el ack del servidor

			socklen_t addrlen= sizeof(remoto);
			bytes = recvfrom(num_socket,datagrama,SIZE_BUFF,0,(struct sockaddr *)&remoto,&addrlen );
			if(bytes <0)
			{
				printf("ERROR en el recvfrom\n");
				exit(ERROR);
			}

			if(modo_v==TRUE)
				printf("Recibido el ack del servidor tftp %s \n",argv[1]);

			//Comprobamos si ha devuelto un mensaje de error
			opcode=datagrama[1];
			if(opcode==5)
			{

				errcode=datagrama[3];
				strcat(errmensaje,datagrama+4);
				printf("Errcode %d: %s\n",errcode,errmensaje);
				exit(ERROR);

			}

			if((primer_datagrama==FALSE) && (tam_envio<SIZE_BUFF))
			{
				if(modo_v==TRUE)
					printf("El bloque %d era el ultimo. Cerramos el fichero\n", bloque);
				//Cerramos el fichero

				if(fclose(fich)==EOF)
				{
					printf("Error a la hora de cerrar el fichero\n");
					exit(ERROR);
				}
				fin=TRUE;
			}
			bloque++;
			primer_datagrama=FALSE;


		}

	}

	else
	{

		printf("Sintaxis: ./tftp-client ip-servidor {-r|-w} archivo [-v]\n");
		close(num_socket);
		exit(ERROR);
	}

	//cerramos el socket


	close(num_socket);
	return 0;

}
