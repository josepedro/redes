/* Lab Redes II - Prof. Fernando W. Cruz */
/* Codigo: tcpClient2.c			     */
/* ***************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#define MAX_SIZE    	80

void message_args_error(int argc, char **argv){
	if(argc<3)  {
	   printf("uso correto: %s <ip_do_servidor> <porta_do_servidor>\n", argv[0]);
	   exit(1);  
	}
}

int create_socket(int af_inet, int sock_stream, int x){
  int socket_description = socket(af_inet, sock_stream, x);
  if (socket_description < 0) {
    fprintf(stderr, "Falha ao criar socket!\n");
    exit(1); 
  }
  return socket_description; 
}

void message_connection_error(int connection_result){
	if (connection_result < 0) {
		fprintf(stderr,"Tentativa de conexao falhou!\n");
		exit(1); 
	}
}

int main(int argc, char * argv[]) {
	struct  sockaddr_in address_server; /* server descriptions */
	int     socket_description;          	      /* socket descriptor              */
	int     n,k;                  /* num caracteres lidos do servidor */
	char    bufout[MAX_SIZE];     /* buffer de dados enviados  */
	char *ip_server = argv[1];
	char *port_server = argv[2];

	/* check arguments */
	message_args_error(argc, argv);

	/* Create socket */	
	socket_description = create_socket(AF_INET, SOCK_STREAM, 0);

	/* clean variables */
	memset((char *)&address_server,0,sizeof(address_server)); /* limpa estrutura */
	memset((char *)&bufout,0,sizeof(bufout));     /* limpa buffer */

	/* set configurations address server */
	address_server.sin_family      = AF_INET; /* config. socket p. internet*/
	address_server.sin_addr.s_addr = inet_addr(ip_server);
	address_server.sin_port        = htons(atoi(port_server));

	/* Connect socket to definied server */
	int connection_result = connect(socket_description, (struct sockaddr *)&address_server, sizeof(address_server));
	message_connection_error(connection_result); 

	while (1) {
		printf("> ");
		fgets(bufout, MAX_SIZE, stdin);    /* le dados do teclado */
		printf("%s\n", bufout);
		send(socket_description,&bufout,strlen(bufout),0); /* enviando dados ...  */
		if (strncmp(bufout, "FIM",3) == 0) 
			break;
	} /* fim while */
	printf("------- encerrando conexao com o servidor -----\n");
	close (socket_description);
	return (0);
} /* fim do programa */

