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

int desarrollado=0;


int main(int argc, char *argv[]){


    if(argc==2 || argc==3){
        if(argc==2){

        }else{

        }
    }else{
        fprintf(stderr,"uso: %s ip-servidor [-v]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

//**********************************************************************************

ECHORequest echoRequest;
ECHOResponse echoResponse;





    return(EXIT_SUCCESS);
}
