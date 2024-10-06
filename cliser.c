#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

#define numMaq 3

char *gets(char *);

typedef struct {
    int metricaUm;
    int metricaDois;
    int metricaTres;
    int maqId;
} Ebpf;

Ebpf maquinas[numMaq];
int tempo = 10;
pthread_mutex_t tempo_mutex = PTHREAD_MUTEX_INITIALIZER;

Ebpf functionebpf() {
    // Inicializando a semente do gerador de números aleatórios
    srand(time(0));

    // Criando a variável do tipo TresNumeros
    Ebpf numeros;
    
    // Gerando os três números aleatórios
    numeros.metricaUm = rand() % 100;  // Número entre 0 e 99
    numeros.metricaDois = rand() % 100;  // Número entre 0 e 99
    numeros.metricaTres = rand() % 100;  // Número entre 0 e 99

    return numeros;  // Retorna a struct com os números
}

void* mainCliente(void* arg) {

    int sock;
    struct sockaddr_in target;
    char **argv = (char**)arg;
    char *servIP[numMaq-1];
    Ebpf metricas;
    // Consistencia
    for (int i = 0; i < numMaq - 1; i++) {
        servIP[i] = argv[i + 1];
    }
    // Criando Socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        printf("Socket Falhou!!!\n");

    do {
        metricas = functionebpf();
        metricas.maqId = atoi(argv[numMaq]);
        for (int i = 0; i < numMaq - 1; i++) {
            // Construindo a estrutura de endereco do servidor
            memset(&target, 0, sizeof(target));
            target.sin_family = AF_INET;
            target.sin_addr.s_addr = inet_addr(servIP[i]); // host local
            target.sin_port = htons(6000); // porta de destino
            
            // Envia mensagem para o endereco remoto
            sendto(sock, &metricas, sizeof(Ebpf), 0, (struct sockaddr *)&target, sizeof(target));
            maquinas[metricas.maqId - 1] = metricas;
        }
        sleep(tempo);
    } while (1);
    
    close(sock);
    return NULL;
}

void* mainServidor(void* arg) {
    char **argv = (char**)arg;
    int sock;
    struct sockaddr_in me, from;
    socklen_t adl = sizeof(from);
    Ebpf receivedMetricas;
    // Cria o socket para enviar e receber datagramas
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        printf("ERRO na Criacao do Socket!\n");
    else  
        printf("Esperando Mensagens...\n");

    // Construcao da estrutura do endereco local
    memset(&me, 0, sizeof(me));
    me.sin_family = AF_INET;
    me.sin_addr.s_addr = htonl(INADDR_ANY); // endereco IP local
    me.sin_port = htons(6000); // porta local

    // Bind para o endereco local
    if (bind(sock, (struct sockaddr *)&me, sizeof(me)) != -1)
        do {
            // Recebe mensagem do endereco remoto
            recvfrom(sock, &receivedMetricas, sizeof(Ebpf), 0, (struct sockaddr *)&from, &adl);
            maquinas[receivedMetricas.maqId-1] = receivedMetricas;
        } while (1);
    else 
        puts("Porta ocupada");
    
    close(sock);
    return NULL;
}

void* dashboard(void* arg) {
    do {
        system("clear");
        printf("------------------------------------------------------\n");
        printf("          Estatística das máquinas                    \n");
        printf("------------------------------------------------------\n");
        for (int i = 0; i < numMaq; i++) {
            printf("Maquina %d:\n\nMétrica 1: %d\tMétrica 2: %d\tMétrica 3: %d \n\n", i + 1,
            maquinas[i].metricaUm, maquinas[i].metricaDois, maquinas[i].metricaTres);   
        }
        sleep(1);
    } while (1);
}

void* mudarTempoCli(void* arg) {

	int sock,i;
	struct sockaddr_in target;
    char linha[10];
	char *servIP[numMaq-1];	
    char **argv = (char **)arg;
	for (int i = 0; i < numMaq - 1; i++) {
        servIP[i] = argv[i + 1];
    }

	/* Criando Socket */
	if ((sock = socket(AF_INET,SOCK_DGRAM,0)) < 0)
    		printf("Socket Falhou!!!\n");
		

	do {
        gets(linha); // irah gerar um warning de unsafe/deprecated.
        pthread_mutex_lock(&tempo_mutex);
        tempo = atoi(linha);
        pthread_mutex_unlock(&tempo_mutex);
        for (int i = 0; i < numMaq - 1; i++) {
            /* Construindo a estrutura de endereco do servidor
            A funcao bzero eh usada para colocar zeros na estrutura target */
            bzero((char *)&target,sizeof(target));
            target.sin_family = AF_INET;
            target.sin_addr.s_addr = inet_addr(servIP[i]); /* host local */
            target.sin_port = htons(6001); /* porta de destino */
            /* Envia mensagem para o endereco remoto
            parametros(descritor socket, dados, tamanho dos dados, flag, estrutura do socket remoto, tamanho da estrutura) */
            sendto(sock, linha, 10, 0, (struct sockaddr *)&target, sizeof(target));
        }
    } while(strcmp(linha,"exit"));
    close(sock);
    return 0;
}

void* mudarTempoSer(void* arg) {

	int sock;
	/* Estrutura: familia + endereco IP + porta */
	struct sockaddr_in me, from;
    // socklen_t for MAC
	int adl = sizeof(me);
	char linha[10];

   	/* Cria o socket para enviar e receber datagramas
	par�metros(familia, tipo, protocolo) */
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        	printf("ERRO na Criacao do Socket!\n");
	else  
		//printf("Esperando Mensagens...\n");

	/* Construcao da estrutura do endereco local
	Preenchendo a estrutura socket me (familia, IP, porta)
	A funcao bzero eh usada para colocar zeros na estrutura me */
	bzero((char *)&me, sizeof(me));
	me.sin_family = AF_INET;
	me.sin_addr.s_addr=htonl(INADDR_ANY); /* endereco IP local */
	me.sin_port = htons(6001); /* porta local  */

   	/* Bind para o endereco local
	parametros(descritor socket, estrutura do endereco local, comprimento do endereco) */
	if(-1 != bind(sock, (struct sockaddr *)&me, sizeof(me)))
	do  {
		/* Recebe mensagem do endereco remoto
		parametros(descritor socket, dados, tamanho dos dados, flag, estrutura do socket remoto, tamanho da estrutura) */
		recvfrom(sock, linha, 10, 0, (struct sockaddr *)&from, &adl);
        pthread_mutex_lock(&tempo_mutex);
        tempo = atoi(linha);
        pthread_mutex_unlock(&tempo_mutex);
	}
	while(strcmp(linha,"exit"));
    else puts("Porta ocupada");
	close(sock);
    return 0;
}

int main(int argc, char *argv[]) {
    pthread_t tid[4];
    void* statusCli;
    void* statusSer;
    void* statusDas;
    void* statusTemC;
    void* statusTemS;

    srand((long)time(NULL));

    // Criando duas threads
    // Cliente
    if (pthread_create(tid + 0, 0, mainCliente, (void*)argv) != 0) { 
        perror("Erro ao criar thread do cliente...."); 
        exit(1); 
    } 
    // Servidor
    if (pthread_create(tid + 1, 0, mainServidor, (void*)argv) != 0) { 
        perror("Erro ao criar thread do servidor...."); 
        exit(1); 
    } 

    //dashboard
    if (pthread_create(tid + 2, 0, dashboard, NULL) != 0) { 
        perror("Erro ao criar thread do servidor...."); 
        exit(1); 
    } 

    // //Mudar o tempo cliente
    if (pthread_create(tid + 3, 0, mudarTempoCli, (void*)argv) != 0) { 
        perror("Erro ao criar thread do servidor...."); 
        exit(1); 
    } 

    //Mudar o tempo servidor
    if (pthread_create(tid + 4, 0, mudarTempoSer, NULL) != 0) { 
        perror("Erro ao criar thread do servidor...."); 
        exit(1); 
    } 

    // Esperando as threads terminarem
    if (pthread_join(tid[0], &statusCli) != 0) { 
        perror("Erro no pthread_join() do cliente."); 
        exit(1); 
    } 

    if (pthread_join(tid[1], &statusSer) != 0) { 
        perror("Erro no pthread_join() do servidor."); 
        exit(1); 
    } 

    
    if (pthread_join(tid[2], &statusDas) != 0) { 
        perror("Erro no pthread_join() do servidor."); 
        exit(1); 
    } 

    if (pthread_join(tid[3], &statusTemC) != 0) { 
        perror("Erro no pthread_join() do servidor."); 
        exit(1); 
    } 

    if (pthread_join(tid[4], &statusTemS) != 0) { 
        perror("Erro no pthread_join() do servidor."); 
        exit(1); 
    } 

    return 0;
}

