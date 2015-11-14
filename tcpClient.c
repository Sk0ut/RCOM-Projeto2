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

#define DEBUG

#define SERVER_PORT 21 /* FTP Port */
#define MAX_STRING_SIZE	255
#define IP_MAX_SIZE	15

/* TODO: Isto vai muito provavelmente ser passado para um .h */
typedef struct {
	char server_adress[IP_MAX_SIZE]; //NNN.NNN.NNN.NNN
	int socketfd;
	char username[MAX_STRING_SIZE];
	char password[MAX_STRING_SIZE];
} ftp_t;

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

ftp_t* ftp_init(char* server, char* username, char* password){
	
	ftp_t* ftp = malloc(sizeof(ftp_t));
	memset(ftp, 0, sizeof(ftp_t));
	char server_adress[IP_MAX_SIZE];

	if(getIpAdress(server, server_adress) < 0){
		printf("Error while resolving adress %s\n", server);
		return NULL;
	}

	strcpy(ftp->server_adress, server_adress);
	strcpy(ftp->username, username);
	strcpy(ftp->password, password);

	return ftp;
}

int ftp_connect(ftp_t* ftp){
	int	sockfd;
	struct	sockaddr_in server_addr;
	
	#ifdef DEBUG
	printf("Starting service handling\n");
	#endif

	/*server address handling*/
	memset((char*)&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ftp->server_adress);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(SERVER_PORT);		/*server TCP port must be network byte ordered */
    
    #ifdef DEBUG
	printf("Finished service handling\n");
	#endif

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
			return -1;
    }

    ftp->socketfd = sockfd;

    #ifdef DEBUG
    printf("Finished opening TCP Socket \n");
    #endif
	/*connect to the server*/
    	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        	perror("connect()");
		return -1;
	}

	char answer[MAX_STRING_SIZE];
	ftp_read_answer(ftp, answer, MAX_STRING_SIZE);
   	
   	#ifdef DEBUG
   	printf("Finished connecting to the server\n");
   	#endif

   	/*send a string to the server*/
	/*int bytes = write(sockfd, "cenas", strlen("cenas"));
	printf("Bytes escritos %d\n", bytes);*/
	return sockfd;
}

int ftp_send_command(ftp_t* ftp, char* command, int size){
	int bytesSent = write(ftp->socketfd, command, size);

	if(bytesSent <= 0){
		printf("Error while sending command to server (no information sent)\n");
		return -1;
	}

	if(bytesSent != size){
		printf("Error while sending command to server (information partially sent)\n");
		return -1;
	}

	#ifdef DEBUG
		printf("Sent command \"%s\", %d bytes written\n", command, bytesSent);
	#endif

	return 0;
}

int ftp_read_answer(ftp_t* ftp, char* answer, int size){	
	int bytesRead;
	memset(answer, 0, size);
	bytesRead = read(ftp->socketfd, answer, size);
	printf("Answer: %s\n", answer);
	return 0;
}

int ftp_login_host(ftp_t* ftp){

	char command[4 + strlen(ftp->username)];
	char answer[MAX_STRING_SIZE];

	sprintf(command,"%s %s", "user", ftp->username);
	ftp_send_command(ftp, command, strlen(command));
	ftp_read_answer(ftp, answer, MAX_STRING_SIZE);

	sprintf(command,"%s %s", "pass", ftp->password);
	ftp_send_command(ftp, command, strlen(command));
	ftp_read_answer(ftp, answer, MAX_STRING_SIZE);
	return 0;
}

int ftp_delete(ftp_t* ftp){
	if(close(ftp->socketfd) < 0){
		printf("Failed to close ftp socket.\n");
		return -1;
	}
	free(ftp);
	return 0;
}


int main(int argc, char** argv){
	if(argc != 4){
		printf("Usage: %s <server> <username> <password>", argv[0]);
		return 1;
	}

	ftp_t* ftp = ftp_init(argv[1], argv[2], argv[3]);
	if(ftp == NULL)
		return -1;

	if(ftp_connect(ftp) < 0)
		return -1;

	if(ftp_login_host(ftp) < 0)
		return -1;

	if(ftp_delete(ftp) < 0)
		return -1;

	return 0;
}