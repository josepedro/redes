#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define MAX_SIZE    	80

void threadSend(int *socket_description)
{
	char    bufout[MAX_SIZE];

	while (1) {
		memset(&bufout, 0x0, sizeof(bufout));
		printf("EU# Digite algo (FIM - para terminar): ");
		fgets(bufout, MAX_SIZE, stdin);
		send(*socket_description,&bufout,strlen(bufout),0);
		if (strncmp(bufout, "FIM",3) == 0) 
			break;
	} 
}

void threadReceiver(int *socket_description)
{
	char	bufin[MAX_SIZE];  /* buffer p receber dados */

	while (1) {
		memset(&bufin, 0x0, sizeof(bufin));
		recv(*socket_description, &bufin, sizeof(bufin), 0);
		if (strncmp(bufin, "FIM", 3) == 0)
			break;
		printf("ELE# ");
		printf("<- %s", bufin);
	}
}

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

void intialize_threads_comunication(int socket_description){
	pthread_t pth_send;
	pthread_t pth_receiver;
	int *socket_description_pointer;
	socket_description_pointer = &socket_description;
	pthread_create(&pth_send,NULL,(void *) threadSend, socket_description_pointer);
	pthread_create(&pth_receiver,NULL,(void *) threadReceiver, socket_description_pointer);
	pthread_join(pth_send,NULL);
	pthread_join(pth_receiver,NULL);
}

int main(int argc,char * argv[]) {
	struct  sockaddr_in address_server; /* server descriptions */
	int     socket_description; /* socket descriptor */

  	message_args_error(argc, argv);

	memset((char *)&address_server,0,sizeof(address_server)); /* clean address server struct */
	address_server.sin_family      = AF_INET; /* configuration socket to internet*/
	address_server.sin_addr.s_addr = inet_addr(argv[1]); /* ip server */
	address_server.sin_port        = htons(atoi(argv[2])); /* port server */

  	socket_description = create_socket(AF_INET, SOCK_STREAM, 0);

	int connection_result = connect(socket_description, (struct sockaddr *)&address_server, sizeof(address_server));
	message_connection_error(connection_result); 

	intialize_threads_comunication(socket_description);

	printf("------- finishing connections -----\n");
	close (socket_description); /* fecha a conexao */
	return (0);
} /* fim do programa */

