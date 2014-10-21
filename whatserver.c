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
#include <pthread.h>

#define QLEN	5               /* tamanho da fila de clientes  */
#define MAX_SIZE	80		/* tamanho do buffer */
#define ROOM_SIZE	2

typedef struct Item{
	void * data;
	struct Item * next;
	struct Item * prev;
}Item;

typedef struct Queue{
	Item * first;
	Item * last;
	int size;
}Queue;

typedef struct Client{
	char * name;
	int s_descriptor;
	int busy;
}Client;

typedef struct Room{
	Queue * clientQueue;
	char * name;
}Room;

typedef struct thread_args{
	Client * client;
	Room * room;
}thread_args;

typedef struct ClientList{

}ClientList;

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

void * clientListener(Client * client, Room * room);
static Client server = {"[Server]",0,1};
static Item placeholder = {NULL,NULL,NULL};
static Queue * lobbyQueue;
static Queue * roomQueue;
static Queue * clientMainQueue;


Queue * initQueue(){
	Queue * queue = (Queue *) malloc(sizeof(Queue));
	queue->last = &placeholder;
	queue->first = &placeholder;
	queue->size = 0;
	return queue;
}

void queue(Queue * queue, void * data){
	if(queue->first == &placeholder){
		Item * item = (Item *) malloc(sizeof(Item));
		item->next = &placeholder;
		item->prev = &placeholder;
		item->data = data;
		queue->first = item;
		queue->last = item;
		queue->size++;
	}else{
		Item * item = (Item *) malloc(sizeof(Item));
		item->next = queue->last;
		item->prev = queue->last->prev;
		item->data = data;
		queue->last->prev = item;
		queue->size++;
	}
}

void * dequeue(Queue * queue){
	Item * first = queue->first;
	queue->first = queue->first->prev;
	queue->first->next = first->next;
	queue->size--;
	return first->data;
}

void removeByData(Queue * queue, void * data){
	Item * iterator =  queue->last;
	while(iterator != &placeholder){
		if(iterator->data == data){
			iterator->prev->next = iterator->next;
			iterator->next->prev = iterator->prev;
			queue->size--;
			free(iterator);
			break;
		}
		iterator = iterator->next;
	}
}

char * concat(char * s1, char * s2){
	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	char * result = malloc(len1+len2+1);
	memcpy(result, s1, len1);
	memcpy(result+len1,s2, len2+1);
	return result;
}

int broadcast(Room * room, Client * client, char * message){
	Item * iterator = room->clientQueue->last;
	char * output_buffer;
	output_buffer = concat(client->name,": ");
	output_buffer = concat(output_buffer, message);
	while(iterator != &placeholder){
		Client * reciever = (Client *)iterator->data;
		send(reciever->s_descriptor,output_buffer,strlen(output_buffer),0);
		iterator = iterator->next;
	}
	return 0;
}

int multicast(Room * room, Client * client, char * message){
	Item * iterator = room->clientQueue->last;
	char * output_buffer;
	output_buffer = concat(client->name,": ");
	output_buffer = concat(output_buffer, message);
	while(iterator != &placeholder){
		Client * reciever = (Client *)iterator->data;
		if(client->s_descriptor !=  reciever->s_descriptor){
			send(reciever->s_descriptor,output_buffer,strlen(output_buffer),0);
		}
		iterator = iterator->next;
	}
	return 0;
}


void redirectToLobby(Client * client){
	client->busy = 0;
	queue(lobbyQueue, client);
}

void showCommands(Client * client){
	char message[MAX_SIZE];
	memset(&message,0x0, sizeof(message));
	sprintf(message, "1 - Listar Salas | 2 - Entrar na sala (#)\n");
	send(client->s_descriptor,message,strlen(message),0);
	memset(&message,0x0, sizeof(message));
	sprintf(message, "3 - Criar Sala | 4 - Desconectar\n");
	send(client->s_descriptor,message,strlen(message),0);
}

void * clientListener(Client * client, Room * room){
	queue(room->clientQueue,client);
	client->busy = 1;
	char input_buffer[MAX_SIZE];
	char message[MAX_SIZE];
	int r;
	while(1){
		memset(&input_buffer, 0x0, sizeof(input_buffer));
    	r = recv(client->s_descriptor, &input_buffer, sizeof(input_buffer),0);
		if (strncmp(input_buffer, "exit()", 6) == 0){
			break;
		}
		multicast(room,client,strdup(input_buffer));
	}
	memset(&message, 0x0, sizeof(message));
	strcpy(message, client->name);
	strcat(message, " saiu da sala!\n");
	broadcast(room,&server,strdup(message));
	removeByData(room->clientQueue,client);
}

void * actionSelector(void *arg){
	Client * client = (Client *)arg;
	char message[MAX_SIZE];
	char input_buffer[MAX_SIZE];
	int r, index = 0;
	Item * iterator = &placeholder;
	int exit_call = 0;
	memset(&message,0x0, sizeof(message));
	sprintf(message, "Bem vindo ao Lobby!\n");
	send(client->s_descriptor,message,strlen(message),0);
	memset(&message,0x0, sizeof(message));
	sprintf(message, "Informe seu nome:\n");
	send(client->s_descriptor,message,strlen(message),0);
	memset(&input_buffer, 0x0, sizeof(input_buffer));
	r = recv(client->s_descriptor, &input_buffer, sizeof(input_buffer),0);
	strtok(input_buffer, "\n");
	client->name = strdup(input_buffer);
	do{
		showCommands(client);
		memset(&input_buffer, 0x0, sizeof(input_buffer));
		r = recv(client->s_descriptor, &input_buffer, sizeof(input_buffer),0);
		if(strncmp(input_buffer,"1",1)==0){
			iterator = roomQueue->last;
			index = 0;
			while(iterator != &placeholder){
				memset(&message,0x0, sizeof(message));
				sprintf(message, "[%d] %s\n",index,((Room *)(iterator->data))->name);
				send(client->s_descriptor,message,strlen(message),0);
				index++;
				iterator = iterator->next;
			}

		}else if(strncmp(input_buffer,"2",1)==0){
			memset(&message,0x0, sizeof(message));
			sprintf(message, "Informe o número da sala:\n");
			send(client->s_descriptor,message,strlen(message),0);
			memset(&input_buffer, 0x0, sizeof(input_buffer));
			r = recv(client->s_descriptor, &input_buffer, sizeof(input_buffer),0);
			strtok(input_buffer, "\n");
			char * num;
			long position = strtol(input_buffer, &num, 0);
			iterator = roomQueue->last;
			for(index = 0; index < position; index++){
				iterator = iterator->next;
			}
			Room * room = ((Room *)(iterator->data));
			clientListener(client, room);
		}else if(strncmp(input_buffer,"3",1)==0){
			memset(&message,0x0, sizeof(message));
			sprintf(message, "Informe o nome da sala:\n");
			send(client->s_descriptor,message,strlen(message),0);
			memset(&input_buffer, 0x0, sizeof(input_buffer));
			r = recv(client->s_descriptor, &input_buffer, sizeof(input_buffer),0);
			strtok(input_buffer, "\n");
			Room * room = (Room *) malloc(sizeof(Room));
			room->clientQueue = initQueue();
			room->name = strdup(input_buffer);
			queue(roomQueue,room);
			clientListener(client, room);
		}else if(strncmp(input_buffer,"4",1)==0){
			exit_call = 1;
			memset(&message,0x0, sizeof(message));
			sprintf(message, "Desconectando do sistema!\n");
			send(client->s_descriptor,message,strlen(message),0);
			close(client->s_descriptor);
		}else{
			memset(&message,0x0, sizeof(message));
			sprintf(message, "Opção Inválida!\n");
			send(client->s_descriptor,message,strlen(message),0);
		}
	}while(!exit_call);
}

void * runLobby(void * arg){
	Queue * roomQueue = initQueue();
	Queue threadQueue = {&placeholder, &placeholder, 0};
	// CHANGE THIS CONDITION (INIFITE LOOP);
	while(1){
		while(lobbyQueue->size != 0){
			pthread_t thread;
			Client * client = dequeue(lobbyQueue);
			pthread_create(&thread, NULL, actionSelector, client);
			queue(&threadQueue, &thread);
		}

	}
	while((&threadQueue)->size != 0){
		pthread_join(*((pthread_t *)dequeue(&threadQueue)), NULL);
	}
}

int main(int argc, char *argv[]) {

  struct sockaddr_in address_server;  /* endere�o do servidor   */
  struct sockaddr_in address_client;   /* endere�o do cliente    */
  int    socket_description, new_socket_description;          /* socket descriptors */
  int    pid, size_address_client, n;
  char *ip_server = argv[1];
  char *port_server = argv[2];

  //Loading Queues
  lobbyQueue = initQueue();
  roomQueue =  initQueue();
  clientMainQueue =  initQueue();

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

  pthread_t thread;

  pthread_create(&thread, NULL, runLobby, NULL);

  for ( ; ; ) {
	  /* waiting a new process client to connect and accept */
    printf("%s\n","Procurando aceitar o cliente");
    new_socket_description = accept(socket_description, (struct sockaddr *)&address_client,(socklen_t*) &size_address_client);
	  printf("%s\n","Cliente aceitado");
    message_accept_socket_error(new_socket_description);
	printf("%s\n", inet_ntoa(address_client.sin_addr));

	fprintf(stdout, "Cliente %s: %u conectado.\n", inet_ntoa(address_client.sin_addr), ntohs(address_client.sin_port));

	Client * client = (Client *)malloc(sizeof(Client));
	client->name = NULL;
	client->busy = 0;
	client->s_descriptor = new_socket_description;
	queue(lobbyQueue, client);
  }
  pthread_join(thread, NULL);
}