#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#define ECHOMAX 255

int main(void) {

	int sock;
	/* Estrutura: familia + endereco IP + porta */
	struct sockaddr_in me, from;
    // socklen_t for MAC
	int adl = sizeof(me);
	char linha[ECHOMAX];

   	/* Cria o socket para enviar e receber datagramas
	par�metros(familia, tipo, protocolo) */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        	printf("ERRO na Criacao do Socket!\n");
	else  
		printf("Esperando Mensagens...\n");

	/* Construcao da estrutura do endereco local
	Preenchendo a estrutura socket me (familia, IP, porta)
	A funcao bzero eh usada para colocar zeros na estrutura me */
	bzero((char *)&me, sizeof(me));
	me.sin_family = AF_INET;
	me.sin_addr.s_addr=htonl(INADDR_ANY); /* endereco IP local */
	me.sin_port = htons(6000); /* porta local  */

   	/* Bind para o endereco local
	parametros(descritor socket, estrutura do endereco local, comprimento do endereco) */
	if(-1 != bind(sock, (struct sockaddr *)&me, sizeof(me)))
	do  {
		/* Recebe mensagem do endereco remoto
		parametros(descritor socket, dados, tamanho dos dados, flag, estrutura do socket remoto, tamanho da estrutura) */
		recvfrom(sock, linha, ECHOMAX, 0, (struct sockaddr *)&from, &adl);
        printf("%s\n", linha);
	}
	while(strcmp(linha,"exit"));
    else puts("Porta ocupada");
	close(sock);
    return 0;
}
