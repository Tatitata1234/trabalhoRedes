#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#define ECHOMAX 255

int main(int argc, char *argv[]) {

	int sock,i;
	struct sockaddr_in target;
    char linha[ECHOMAX+1];
	char *servIP;	
   
	/* Consistencia */
	if (argc != 2) {	
		fprintf(stderr,"Usage: %s <Server IP> \n",argv[0]);
		exit (1);
	}
	servIP = argv[1];

	/* Criando Socket */
	if ((sock = socket(AF_INET,SOCK_DGRAM,0)) < 0)
    		printf("Socket Falhou!!!\n");
		
	/* Construindo a estrutura de endereco do servidor
	A funcao bzero eh usada para colocar zeros na estrutura target */
    bzero((char *)&target,sizeof(target));
    target.sin_family = AF_INET;
    target.sin_addr.s_addr = inet_addr(servIP); /* host local */
    target.sin_port = htons(6000); /* porta de destino */

	do {
        gets(linha); // irah gerar um warning de unsafe/deprecated.
		/* Envia mensagem para o endereco remoto
		parametros(descritor socket, dados, tamanho dos dados, flag, estrutura do socket remoto, tamanho da estrutura) */
        sendto(sock, linha, ECHOMAX, 0, (struct sockaddr *)&target, sizeof(target));
    } while(strcmp(linha,"exit"));
    close(sock);
    return 0;
}
