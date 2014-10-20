#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

//Definições
#define MAXIMO 1024

//Variá¡veis Globais
int port, tam, tam1;
char *nome, *nome1, *ip;
pthread_t t2, t3;

//Funções
void* Tm()									//Funções utilizada na thread para obtenção de data e hora
{
	struct tm *local;
	time_t   tempo;
	tempo  = time(NULL);
	local  = localtime(&tempo);
 
	printf("%d/%d/%d- %dh%dm%ds",local->tm_mday, local->tm_mon + 1, local->tm_year + 1900, local->tm_hour, local->tm_min, local->tm_sec);

	pthread_exit(NULL);
}

void Enviar(int socket_fd)							//Funções utilizada pelo servidor para comunicação, leitura e escrita
{
	int  length;
	char text[MAXIMO];
	pthread_t t1;

	while(1)
	{
		//Procedimento de escrita
		memset(text, 0, MAXIMO);
		length = 0;

		while((text[length++]=getchar()) != '\n'); 
		write(socket_fd, text, sizeof(text));
		text[strlen(text)-1] = '\0';	

		//Teste de termino da conexÃ£o
		if(strncmp(text, "quit.", 5) == 0)
		{
			printf("\nEncerrando conexao...\n\n");
			close(socket_fd);
			sleep(3);
			system("clear");
			exit(0);
		}

		printf("\t\t\t\t\tMensagem enviada - ");
		pthread_create(&t1, NULL, (void *) Tm, NULL);			//Atualizando horario
		pthread_join(t1, NULL);
		printf("\n");
	}
}

void Receber(int socket_fd)							//Funções utilizada pelo servidor para comunicaÃ§Ã£o, leitura e escrita
{
	int  length;
	char text[MAXIMO];
	pthread_t t1;

	//Recebendo nome
	read(socket_fd, &tam1, sizeof(tam1));
	nome1 = (char *)calloc(tam, sizeof(char));
	read(socket_fd, nome1, tam1);

	while(1)
	{

		//Procedimento de leitura
		memset(text, 0, MAXIMO);

		if(read(socket_fd, text, sizeof(text)) == 0)
			pthread_exit(NULL);
		text[strlen(text)-1] = '\0';

		//Teste de termino da conexÃ£o
		if(strncmp(text, "quit.", 5) == 0)
		{
			printf("\nEncerrando conexao...\n\n");
			close(socket_fd);
			sleep(3);
			system("clear");
			exit(0);
		}

		pthread_create(&t1, NULL, (void *) Tm, NULL);			//Atualizando horario
		pthread_join(t1, NULL);	
		printf("  %s --- %s\n", nome1, text);	
	}
}

int main(int argc, char *argv[])
{
	//Argumentos de entrada
	if(argc != 4)
	{
		system("clear");
		printf("Argumentos incorretos. Digite: <NOME> <PORTA> <IP>\n\n");
		exit(1);
	}
		
		//ConversÃ£o de argumentos
		nome = argv[1];
		port = atoi(argv[2]);
		ip   = argv[3];

	//Threads
	pthread_t t1, t2; 

	//Variáveis locais
	int socket_fd, accp;
	struct sockaddr_in server_addr; 

	system("clear");

	//Criando socket
	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Falha na criacao do socket...\n");
		exit(1);
	}
	else
		printf("Socket criado com sucesso..\n");

	//Setando variáveis
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family      = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port        = htons(port);

	//Conectando ao servidor
	if(connect(socket_fd,(struct sockaddr *) &server_addr, sizeof(server_addr)) != 0)
	{
		printf("Falha no connected...\n");
		exit(0);
	}
	else
		printf("Connect executado com sucesso...\n\n");

	printf("Digite quit. para finalizar\n\n");

	//Imprimindo parametros
	printf("IP: %s\n", ip);
	printf("PORTA: %d\n\n", port);

	//Criação das threads	
	printf("==============================================================================\n\n");
	pthread_create(&t2, NULL, (void *)Enviar,  socket_fd);
	pthread_create(&t3, NULL, (void *)Receber, socket_fd);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	//Fechando conexão
	close(socket_fd);

	return 0;
}
