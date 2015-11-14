/*      (C)2000 FEUP  */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <errno.h> 
#include <arpa/inet.h>

#define SERVER_PORT 21 /* FTP Port */
#define DEBUG

int getIpAdress(char* server_addr, char* buf){
	struct hostent *h;

	 if ((h=gethostbyname(server_addr)) == NULL) 
		return -1;

    strcpy(buf, inet_ntoa(*((struct in_addr *)h->h_addr)));

    #ifdef DEBUG
    printf("\nServer info:\n");
    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n\n", buf);
    #endif

    return 0;
}

int ftp_connect(char* server){
	int	sockfd;
	struct	sockaddr_in server_addr;

	char server_adress[16];
	if(getIpAdress(server, server_adress) < 0){
		printf("Error while resolving adress %s\n", server);
		return 1;
	}
	
	#ifdef DEBUG
	printf("Starting service handling\n");
	#endif
	/*server address handling*/
	memset((char*)&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(server_adress);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(SERVER_PORT);		/*server TCP port must be network byte ordered */
    
    #ifdef DEBUG
	printf("Finished service handling\n");
	#endif
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
			return 1;
    	}

    #ifdef DEBUG
    printf("Finished opening TCP Socket \n");
    #endif
	/*connect to the server*/
    	if(connect(sockfd, 
	           (struct sockaddr *)&server_addr, 
		   sizeof(server_addr)) < 0){
        	perror("connect()");
		return 1;
	}
   	
   	#ifdef DEBUG
   	printf("Finished connecting to the server\n");
   	#endif

   	/*send a string to the server*/
	int bytes = write(sockfd, "cenas", strlen("cenas"));
	printf("Bytes escritos %d\n", bytes);

	close(sockfd);
	return 0;
}

int main(int argc, char** argv){
	if(argc != 2){
		printf("Usage: %s <server>", argv[0]);
		return 1;
	}

	return ftp_connect(argv[1]);
}