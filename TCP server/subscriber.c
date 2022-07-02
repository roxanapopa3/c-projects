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
	char type; //tipul pachetului (subscribe,unsubscribe, exit)
	char topic[51];
	int store;
} mesaj_client;

/*Structure used for TCP messages from the server*/
typedef struct tcp_struct {
	char ip[16];
	uint16_t port;
	char type[11];
	char topic[51];
	char payload[1501];
} mesaj_tcp;

//Subscribe function
void subscribe_client(struct mesaj_client pac, char topic[],int store,int sock){
	int sc;
	pac.type='s';
	strcpy(pac.topic,topic);
	pac.store = store;
	sc=send(sock,&pac,sizeof(struct mesaj_client),0);
	DIE(sc < 0, "send");
}
//Unsubscribe function
void unsubscribe_client(struct mesaj_client pac, char topic[], int sock){
	int sc;
	pac.type='u';
	strcpy(pac.topic,topic);
	sc=send(sock,&pac,sizeof(struct mesaj_client),0);
	DIE(sc < 0, "send");
}

int main(int argc, char *argv[]){
	int ret = setvbuf(stdout,NULL,_IONBF,BUFSIZ);
	DIE(ret!=0, "setvbuf");
    int sockfd,flag=1,rc;
	char buffer[100];
	struct mesaj_client pachet;
	/*Initialize TCP socket*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	inet_aton(argv[2], &serv_addr.sin_addr);

	/*Initialize file descriptors*/
	fd_set set;
	fd_set temp_set;
	FD_ZERO(&set);
	FD_SET(STDIN_FILENO, &set);
	FD_SET(sockfd,&set);
	int fdmax = STDIN_FILENO > sockfd ? STDIN_FILENO : sockfd;

	//Connect client to the server
	rc=connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(rc < 0, "connect");

	//Send the client id to the server
	rc=send(sockfd,argv[1],10,0);
	DIE(rc < 0, "send");

	//Deactivate Nagle's algorithm
	ret=setsockopt(sockfd, IPPROTO_TCP,TCP_NODELAY, (char *) &flag, sizeof(int));
	DIE(ret<0, "setsockopt");

    while(1){
		temp_set = set;
		rc=select(fdmax+1, &temp_set, NULL,NULL,NULL);
		DIE(rc < 0, "select");
		char topic[200],command[50];
		int store;

		//Command from STDIN
		if(FD_ISSET(STDIN_FILENO, &temp_set)){
			//Get the command
			memset(buffer,0,100);
			rc=read(STDIN_FILENO, buffer, sizeof(buffer));
			DIE(rc<0, "read");
			//Initialize the packet
			memset(&pachet,0,sizeof(struct mesaj_client));

			//For the 'exit' command, we build an empty packet of type 'e'
			if (strncmp(buffer, "exit", 4) == 0) {
				pachet.type='e';
				rc=send(sockfd,&pachet,sizeof(struct mesaj_client),0);
				DIE(rc < 0, "send");
				break;
			}
			/*For (un)subscribe messages, we get the rest of the data
			from STDIN and build the packet*/
			if(strncmp(buffer,"subscribe",9)==0){
				sscanf(buffer, "%s %s %d",command,topic,&store);
				subscribe_client(pachet,topic,store,sockfd);
				memset(command,0,50);
				rc=recv(sockfd,command,30,0);
				DIE(rc < 0, "recv");
				printf("%s\n",command);
			}
			if(strncmp(buffer, "unsubscribe",11)==0){
				sscanf(buffer, "%s %s", command,topic);
				unsubscribe_client(pachet,topic,sockfd);
				memset(command,0,50);
				rc=recv(sockfd,command,34,0);
				DIE(rc < 0, "recv");
				printf("%s\n",command);
			}
		}
		//Receive messages from the server
		if(FD_ISSET(sockfd,&temp_set)) {
			char buf[sizeof(struct tcp_struct)];
			memset(buf,0,sizeof(struct tcp_struct));
			char int_buf[20];
			int pac_size;
			memset(int_buf,0,20);
			/*Receive the size of the packet first*/
			int rc = recv(sockfd,int_buf,sizeof(int_buf),0);
			DIE(rc < 0, "recv");
			sscanf(int_buf,"%d",&pac_size);
			/*Then the packet*/
			rc = recv (sockfd,buf,pac_size,0);
			DIE(rc < 0, "recv");
			if(rc==0)
				break;
			struct tcp_struct *tcp_pac = (struct tcp_struct*) buf;
			//Print received data
			printf("%s:%u - %s - %s - %s\n",tcp_pac->ip, tcp_pac->port, tcp_pac->topic, tcp_pac->type,tcp_pac->payload);
	}
}
    close(sockfd);

}
