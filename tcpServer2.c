/* Lab Redes II - Prof. Fernando W. Cruz */
/* Codigo: tcpServer2.c			     */
/* ***************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#define QLEN            5               /* tamanho da fila de clientes  */
#define MAX_SIZE	80		/* tamanho do buffer */

void message_args_error(int argc){
  if (argc<3) {
    printf("Digite IP e Porta para este servidor\n");
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

void message_bind_error(bind_result){
   if (bind_result < 0) {
    fprintf(stderr,"Ligacao Falhou!\n");
    exit(1); 
  }
}

void message_listen_error(int listen_result){
  if (listen_result < 0) {
    fprintf(stderr,"Falhou ouvindo porta!\n");
    exit(1); 
  }
}

void message_accept_socket_error(int new_socket_description){
  if (new_socket_description < 0){
    fprintf(stdout, "Falha na conexao\n");
    exit(1); 
  }
}

int atende_cliente(int descritor, struct sockaddr_in address_client)  {
  char bufin[MAX_SIZE];
  int  n;

  while (1) {
    memset(&bufin, 0x0, sizeof(bufin));
    n = recv(descritor, &bufin, sizeof(bufin),0);
  
    if (strncmp(bufin, "FIM", 3) == 0)
      break;

    fprintf(stdout, "[%s:%u] => %s\n", inet_ntoa(address_client.sin_addr), ntohs(address_client.sin_port), bufin);
  }
  
  fprintf(stdout, "Encerrando conexao com %s:%u ...\n\n", inet_ntoa(address_client.sin_addr), ntohs(address_client.sin_port));
  close (descritor);
}

int main(int argc, char *argv[]) {
  
  struct sockaddr_in address_server;  /* endereço do servidor   */
  struct sockaddr_in address_client;   /* endereço do cliente    */
  int    socket_description, new_socket_description;          /* socket descriptors */
  int    pid, size_address_client, n; 
  char *ip_server = argv[1];
  char *port_server = argv[2]; 

  message_args_error(argc);
  
  // Preparing address server
  memset((char *) &address_server, 0, sizeof(address_server)); /* clean variable address_server */
  address_server.sin_family = AF_INET; /* family TCP/IP */
  address_server.sin_addr.s_addr 	= inet_addr(ip_server); /* address IP */
  address_server.sin_port 		= htons(atoi(port_server)); /* port */

  socket_description = create_socket(AF_INET, SOCK_STREAM, 0);  

  /* bind socket to ip and port */
  int bind_result = bind(socket_description, (struct sockaddr *)&address_server, sizeof(address_server));
  message_bind_error(bind_result);

  /* Listen port */
  int listen_result = listen(socket_description, QLEN);
  message_listen_error(listen_result);

  printf("Servidor ouvindo no IP %s, na porta %s ...\n\n", ip_server, port_server);
  
  size_address_client = sizeof(address_client);
  
  for ( ; ; ) {
	  /* waiting a new process client to connect and accept */	
    printf("%s\n","Procurando aceitar o cliente");
    new_socket_description = accept(socket_description, (struct sockaddr *)&address_client,(socklen_t*) &size_address_client);
	  printf("%s\n","Cliente aceitado");
    message_accept_socket_error(new_socket_description);
    
    /*ESSE FPRINTF TA FALHANDO*/
    printf("%s\n", inet_ntoa(address_client.sin_addr));
    fprintf(stdout, "Cliente %s: %u conectado.\n", inet_ntoa(address_client.sin_addr), ntohs(address_client.sin_port)); 
	  atende_cliente(new_socket_description, address_client);
  } 
}