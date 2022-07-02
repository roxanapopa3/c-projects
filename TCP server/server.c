#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include "helpers.h"


/*Structure used for client messages*/
typedef struct mesaj_client {
	char type;  // exit = e, subscribe = s, unsubscribe = u
	char topic[51];
	int store;
} mesaj_client;

/*Structure used for TCP messages from the server*/
typedef struct mesaj_tcp {
	char ip[16];
	uint16_t port;
	char type[11];
	char topic[51];
	char payload[1501];
} msg_tcp;

/*Structure used for UDP messages from the server*/
typedef struct mesaj_udp {
	char topic[50];
	uint8_t type;
	char payload[1501];
} msg_udp;

/*Structure for topics*/
typedef struct topic{
	char nume[51];
	int store;
} topic;

/*Structure for client info*/
typedef struct client{
	char id[10];
	int socket;
	int dim_topics;
	int dim_unsent;
	struct mesaj_tcp unsent[100];
	struct topic topics[100];
	int online;
} client;

/*Function to check if there is an existing client with given id*/
int search_client(struct client *clienti, char client_id[],int fdmax){
	int i;
	for(i=0;i<=fdmax;i++)
		if(strcmp(clienti[i].id,client_id)==0)
			return i;
	return -1;
}
/*Function to add a new client to the clients list*/
void create_client(struct client *clients, int sock, int numClienti, char client_id[]){
	strcpy(clients[numClienti].id,client_id);
	clients[numClienti].socket = sock;
	clients[numClienti].online = 1;
}
/*Function to send all unsent store-and-forward messages*/
void send_unsent_msg(struct client *clienti, int sock){
	int i,sc;
	for(i=0;i<clienti[sock].dim_unsent;i++){
		sc=send(clienti[sock].socket,&clienti[sock].unsent[i],sizeof(struct mesaj_tcp),0);
		DIE(sc<0,"send");
	}
}
/*Function that builds a new TCP message containing the info from the UDP server*/
struct mesaj_tcp buildTCPpack(struct sockaddr_in udp_ad, struct mesaj_udp *send){
	struct mesaj_tcp pack;
	memset(&pack,0,sizeof(struct mesaj_tcp));
	pack.port = htons(udp_ad.sin_port);
	strcpy(pack.ip,inet_ntoa(udp_ad.sin_addr));
	strcpy(pack.topic,send->topic);
	pack.topic[50] = '\0';
	return pack;
}
/*Function which parses the payload from the UDP server
into an int and returns the size of the payload*/
int printInt(struct mesaj_udp *udp,struct mesaj_tcp *tcp){
	uint32_t num = ntohl(*(uint32_t *)(udp->payload +1));
	//Verificam bitul de semn
	if(udp->payload[0]==1)
		num*=-1;
	sprintf(tcp->payload,"%d",num);
	strcpy(tcp->type,"INT");
	return 2*(int)sizeof(num)+1;
}
/*Function which parses the payload from the UDP server
into a short float and returns the size of the payload*/
int printShortFloat(struct mesaj_udp *udp,struct mesaj_tcp *tcp){
	double num = (double)(ntohs(*(uint16_t*)(udp->payload)));
	num/=100;
	/*Checking the number of decimals
	(if none, return an int)*/
	float zecimals = num - (int) num;
	if((1-zecimals)==1){
		int num_int = (int) num;
		sprintf(tcp->payload, "%d",num_int);
		strcpy(tcp->type,"SHORT_REAL");
		return (int)sizeof(num_int);
	}
	else{
		//Else return a float with 2 decimals
		sprintf(tcp->payload, "%.2lf", num);
		strcpy(tcp->type,"SHORT_REAL");
		return (int)sizeof(num);
	}
}
/*Function which parses the payload from the UDP server
into a float and returns the size of the payload*/
int printFloat(struct mesaj_udp *udp,struct mesaj_tcp *tcp){
	double num = ntohl(*(uint32_t *)(udp->payload +1));
	for(int i = 0;i<udp->payload[5];i++)
		num/=10;
	//Check if number is signed
	if(udp->payload[0] == 1)
		num*=-1;
	sprintf(tcp->payload,"%lf",num);
	strcpy(tcp->type,"FLOAT");
	return (int)sizeof(num);
}
/*Function to send TCP messages to client*/
void sendTCPmessage(struct client *clienti, struct mesaj_tcp tcp,int max,int size){
	int sc;
	/*Getting the exact size of the packet*/
	int send_size=sizeof(tcp)+size-1500;
	char buff[20];
	sprintf(buff,"%d",send_size);
	for(int i = 0;i<=max;i++){
		for(int j = 0;j<=clienti[i].dim_topics;j++){
			/*Checking if the client is subscribed to given topic
			and client is online*/
			if(strcmp(clienti[i].topics[j].nume,tcp.topic)==0){
				if(clienti[i].online==1){
					//Send the packet size first
					sc=send(clienti[i].socket,buff,sizeof(buff),0);
					DIE(sc < 0, "send");
					//Then the packet
					sc=send(clienti[i].socket, &tcp, send_size,0);
					DIE(sc < 0, "send");
				}
				/*If client not online, check if the store-and-forward
				option is set*/
				else
				{
					/*If set, keep the packet in queue*/
					if(clienti[i].topics[j].store==1){
						clienti[i].dim_unsent++;
						clienti[i].unsent[clienti[i].dim_unsent]=tcp;
					}
				}
				break;
			}

		}
	}

}
//Subscribe function
void subscribeClient(struct client *client, char topic[],int store){
	//Checking if the client is subscribed to the topic
	for(int i=0;i<=client->dim_topics;i++)
		if(strcmp(client->topics[i].nume,topic)==0)
			return;
	//If not, subscribe
	strcpy(client->topics[client->dim_topics].nume,topic);
	client->topics[client->dim_topics].store = store;
	client->dim_topics++;
}

//Unsubscribe function
void unsubscribeClient(struct client *client,char topic[]){
	for(int i=0;i<client->dim_topics;i++)
	//Checking if the client is subscribed to the topic
		if(strcmp(client->topics[i].nume,topic)==0){
			for(int j=i;j<client->dim_topics;j++)
				client->topics[j] = client->topics[j+1];
			client->dim_topics--;
			return;
		}
}

int main(int argc, char *argv[]){

	int ret=setvbuf(stdout,NULL,_IONBF,BUFSIZ);
	DIE(ret!=0, "setvbuf");
    int tcp_sockfd, newsockfd, udp_sockfd;
	char buffer[sizeof(struct mesaj_client)];
	struct sockaddr_in serv_addr, cli_addr, udp_add;
	int i,flag=1,numClienti=0,nrMaxActual=1000,rc;
	socklen_t clilen;
	char client_id[10];
	char exit_buf[4];

	fd_set read_fds;
	fd_set tmp_fds;
	int fdmax;

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	/*Initialize TCP socket*/
	tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcp_sockfd < 0, "socket");
	/*Initialize UDP socket*/
	udp_sockfd = socket(PF_INET, SOCK_DGRAM,0);
	DIE(udp_sockfd < 0, "socket");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[1]));
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	rc = bind(tcp_sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(rc < 0, "bind");

	udp_add.sin_family = AF_INET;
	udp_add.sin_port = htons(atoi(argv[1]));
	udp_add.sin_addr.s_addr = INADDR_ANY;

	rc=bind(udp_sockfd, (struct sockaddr *)&udp_add, sizeof(struct sockaddr));
	DIE(rc < 0, "bind");

	ret=setsockopt(tcp_sockfd, IPPROTO_TCP, TCP_NODELAY, (char* )&flag,sizeof(int));
	DIE(ret<0, "setsockopt");
	rc=listen(tcp_sockfd, 5);
	DIE(rc < 0, "listen");

	/*Initialize file descriptors*/
	FD_SET(tcp_sockfd, &read_fds);
	FD_SET(udp_sockfd, &read_fds);
	FD_SET(STDIN_FILENO, &read_fds);
	fdmax = tcp_sockfd > udp_sockfd ? tcp_sockfd : udp_sockfd;
	/*Initialize clients list*/
	struct client *clienti = malloc(1000*sizeof(struct client));
	int ok=0;

	socklen_t len = sizeof(struct sockaddr);
    while (!ok) {
		tmp_fds = read_fds;

		rc=select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(rc < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				memset(buffer, 0, sizeof(struct mesaj_client));
				//When connection messages are received
				if (i == tcp_sockfd) {
					//New socket for new client
					clilen = sizeof(cli_addr);
					newsockfd = accept(tcp_sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd<0, "accept");
					//Deactivate Nagle's algorithm
					ret=setsockopt(newsockfd, IPPROTO_TCP,TCP_NODELAY, (char *) &flag, sizeof(int));
					DIE(ret<0, "setsockopt");
					//Receive client id
					rc=recv(newsockfd,client_id,10,0);
					DIE(rc < 0, "recv");
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) {
						fdmax = newsockfd;
					}

					//Check if there is an existing client with given id
					int gasit = search_client(clienti,client_id,fdmax);
					//If there is not, create new client
					if(gasit==-1){
						FD_SET(newsockfd, &read_fds);
						fdmax = newsockfd > fdmax ? newsockfd : fdmax;
						create_client(clienti, newsockfd, numClienti, client_id);
						/*If the number of clients exceeds the limit, raise the limit*/
						if(numClienti>=nrMaxActual){
							nrMaxActual+=100;
							clienti=realloc(clienti,nrMaxActual*(sizeof(struct client)));
						}
						numClienti++;
						printf("New client %s connected from %s:%d\n", client_id, inet_ntoa (cli_addr.sin_addr) ,newsockfd);
					}
					/*If there is an existing client, but it's not online,
                    create a new client*/
					else if(gasit > -1 && clienti[gasit].online==0){

							FD_SET(newsockfd, &read_fds);
							create_client(clienti,newsockfd,gasit,clienti[gasit].id);
							send_unsent_msg(clienti,gasit);
							clienti[gasit].dim_unsent=0;
							printf("New client %s connected from %s:%d\n",clienti[gasit].id, inet_ntoa (cli_addr.sin_addr) ,newsockfd);
						}
					/*If there is an existing client and is online, close
					the connection*/
					else if(gasit > -1 && clienti[gasit].online==1 ){
							FD_CLR(newsockfd,&read_fds);
							close(newsockfd);
							printf("Client %s already connected.\n", clienti[gasit].id);
						}
					}
				//For UDP messages
				else if(i == udp_sockfd){
					//Receive message and parse it
					char buf[1551];
					memset(buf,0,1551);
					rc=recvfrom(udp_sockfd,buf,1551,0, (struct sockaddr *) &udp_add,&len);
					DIE(rc < 0, "recvfrom");
					struct mesaj_udp *udp_send = (struct mesaj_udp *)buf;
					//Build a TCP message with UDP client's info
					struct mesaj_tcp tcp_send = buildTCPpack(udp_add,udp_send);
					int size;
					/*Parse the payload depending on the type
					specified in the packet*/
					switch(udp_send->type){
						case 0:
							size=printInt(udp_send,&tcp_send);
							break;
						case 1:
							size=printShortFloat(udp_send,&tcp_send);
							break;
						case 2:
							size=printFloat(udp_send,&tcp_send);
							break;
						default:
							size=strlen(udp_send->payload);
							strcpy(tcp_send.type, "STRING");
							strcpy(tcp_send.payload, udp_send->payload);
					}
					/*Send the packet*/
					sendTCPmessage(clienti,tcp_send,fdmax,size);

				}
				//For STDIN input
				else if(i== STDIN_FILENO){
					/*If 'exit' command received,
					the server stops*/
					scanf("%s",exit_buf);
					if(strncmp(exit_buf,"exit",4)==0){
						ok=1;
						break;
					}
				}
				/*For subscribe/unsubscribe/exit messages from clients*/
				else{
					//Receive the packet
					memset(buffer,0,sizeof(struct mesaj_client));
					int rc = recv(i,buffer,sizeof(struct mesaj_client),0);
					DIE(rc < 0, "recv");
					if(rc>0){
						//Parse the data
						struct mesaj_client *pac = malloc(sizeof(struct mesaj_client));
						sscanf(buffer,"%c%s",&pac->type,pac->topic);
						struct client* c=NULL;
						//Find the client who sent the packet
						for(int j=0;j<=fdmax;j++)
							if(clienti[j].socket==i){
								c=&clienti[j];
								break;
							}
						char buffer_sub[30];
						char buffer_unsub[34];
						//Complete action
						switch(pac->type){
							case 's':
								/*For subscribe messages, the server
								sends a reply and subscribes the client
								to the topic*/
								strcpy(buffer_sub,"Subscribed to topic.");
								rc=send(c->socket,buffer_sub,30,0);
								DIE(rc < 0, "send");
								subscribeClient(c,pac->topic,pac->store);
								break;
							case 'u':
								/*For unsubscribe messages, the server
								sends a reply and unsubscribes the client
								from the topic*/
								strcpy(buffer_unsub,"Unsubscribed from topic.");
								rc=send(c->socket,buffer_unsub,34,0);
								DIE(rc < 0, "send");
								unsubscribeClient(c,pac->topic);
								break;
							case 'e':
								/*For exit messages, the server
								sends a reply and marks the client as
								inactive*/
									c->online =0;
									c->socket=-1;
									printf("Client %s disconnected.\n",c->id);
									FD_CLR(i,&read_fds);
									close(i);
									break;
							default:
								break;
						}
					}

				}

			}
		}
	}
    close(tcp_sockfd);
	close(udp_sockfd);

}
