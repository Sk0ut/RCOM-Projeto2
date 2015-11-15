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
#include <libgen.h>

#include "ftpClient.h"

/* TODO: Perguntar a prof se e preciso fazer o cwd separadamente, visto que nao e necessario */

int getIpAdress(const char* server_addr, char* buf){
	struct hostent *h;

	 if ((h=gethostbyname(server_addr)) == NULL) 
		return -1;

    strcpy(buf, inet_ntoa(*((struct in_addr *)h->h_addr)));

    #ifdef DEBUG
    printf("\nDEBUG: Server info:\n");
    printf("DEBUG: Host name  : %s\n", h->h_name);
    printf("DEBUG: IP Address : %s\n\n", buf);
    #endif

    return 0;
}

ftp_t* ftp_init(const char* server, const char* username, const char* password, const char* path_to_file){
	
	ftp_t* ftp = malloc(sizeof(ftp_t));
	memset(ftp, 0, sizeof(ftp_t));
	char server_address[IP_MAX_SIZE];

	if(getIpAdress(server, server_address) < 0){
		printf("Error while resolving address %s\n", server);
		return NULL;
	}

	strcpy(ftp->server_address, server_address);
	strcpy(ftp->username, username);
	strcpy(ftp->password, password);
	strcpy(ftp->path_to_file, path_to_file);

	return ftp;
}

int ftp_connect_socket(const char* ip, const int port){
	int	sockfd;
	struct	sockaddr_in server_addr;
	
	#ifdef DEBUG
	printf("DEBUG: Starting server address handling\n");
	#endif

	/*server address handling*/
	memset((char*)&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */
    
    #ifdef DEBUG
	printf("DEBUG: Finished service handling\n");
	#endif

	/*open a TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
			return -1;
    }

    #ifdef DEBUG
    printf("DEBUG: Finished opening TCP Socket \n");
    #endif

    printf("Connecting to %s:%d\n", ip, port);
	/*connect to the server*/
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("connect()");
		return -1;
	}

    #ifdef DEBUG
	printf("DEBUG: Connected\n");
	#endif

	return sockfd;
}

int ftp_connect(ftp_t* ftp){
	int sockfd = ftp_connect_socket(ftp->server_address, FTP_PORT);

	if(sockfd < 0){
		printf("Error while connecting to server\n");
		return -1;
	}

	ftp->socketfd = sockfd;

	char answer[MAX_STRING_SIZE];
	ftp_read_answer(ftp, answer, MAX_STRING_SIZE);
   	
   	#ifdef DEBUG
   	printf("DEBUG: Finished connecting to the server\n");
   	#endif

	return sockfd;
}

int ftp_send_command(ftp_t* ftp, const char* command, const int size){
	#ifdef DEBUG
	printf("DEBUG: Sending command %s", command);
	#endif

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
		printf("DEBUG: Sent command %s, %d bytes written\n", command, bytesSent);
	#endif

	return 0;
}

int ftp_read_answer(ftp_t* ftp, char* answer, const int size){	
	int bytesRead;
	memset(answer, 0, size);
	bytesRead = read(ftp->socketfd, answer, size);
	#ifdef DEBUG
	printf("DEBUG: %s", answer);
	#endif
	return bytesRead;
}

int ftp_login_host(ftp_t* ftp){

	char command[6 + strlen(ftp->username)];
	char answer[MAX_STRING_SIZE];

	sprintf(command,"user %s\n", ftp->username);
	if(ftp_send_command(ftp, command, strlen(command)) < 0){
		printf("Error while sending login information\n");
		return -1;
	}
	if(ftp_read_answer(ftp, answer, MAX_STRING_SIZE) < 0){
		printf("Error while receiving answer from login information \n");
		return -1;
	}

	sprintf(command,"pass %s\n", ftp->password);
	if(ftp_send_command(ftp, command, strlen(command)) < 0){
		printf("Error while sending login information\n");
		return -1;
	}
	if(ftp_read_answer(ftp, answer, MAX_STRING_SIZE) < 0){
		printf("Error while receiving answer from login information \n");
		return -1;
	}

	int replyCode;
	sscanf(answer, "%d", &replyCode);
	if(replyCode == LOGIN_FAIL){ /* Failed to login */
		printf("Wrong username/password combination.\n");
		return -1;
	}

	printf("Logged in to server %s.\n",ftp->server_address);

	return 0;
}

int ftp_set_passive_mode(ftp_t* ftp){
	char answer[MAX_STRING_SIZE];
	int ip1, ip2, ip3, ip4;
	int port1, port2;
	if(ftp_send_command(ftp, "pasv\n", strlen("pasv\n")) < 0){
		printf("Error while switching to passive mode\n");
		return -1;
	}
	if(ftp_read_answer(ftp, answer, MAX_STRING_SIZE) < 0){
		printf("Error while receiving answer from switching to passive mode \n");
		return -1;
	}

	#ifdef DEBUG
		printf("%s", answer);
	#endif

	if ((sscanf(answer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &port1, &port2)) < 0){
		printf("Error while parsing pasv answer from the server.\n");
		return -1;
	}

	char ip[15];
	int port = 256*port1+port2;
	sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);

	#ifdef DEBUG
	printf("IP: %s Port: %d\n", ip, port);
	#endif

	int datafd = ftp_connect_socket(ip, port);
	if(datafd < 0){
		printf("Error while connecting to data socket\n");
		return -1;
	}

	ftp->datafd = datafd;
	return 0;
}

int ftp_download_file(ftp_t* ftp){

	char filename[MAX_STRING_SIZE];
	char buf[DATA_PACKET_SIZE];
	sprintf(filename, "%s", basename(ftp->path_to_file));
	int bytesRead = 0;
	int totalBytes = 0;

	FILE* file = fopen(filename, "w");
	if(file == NULL){
		printf("Couldn't open file %s for writing\n", filename);
		return -1;
	}

	while((bytesRead = read(ftp->datafd, buf, sizeof(buf))) != 0){	
		printf("Downloading file... %.2f%% complete.\n", (100.0 * totalBytes) / ftp->file_size);
		if(bytesRead < 0){
			printf("Error in reading file from server\n");
			return -1;
		}
		totalBytes += bytesRead;
		if(fwrite(buf,sizeof(char),bytesRead,file) == 0)
			if(ferror(file) != 0){
				printf("Error in writing to file %s\n", filename);
				return -1;
			}
	}
	printf("File downloaded	\n");
	fclose(file);
	return totalBytes;
}

int ftp_retr_file(ftp_t* ftp){
	char command[6+strlen(ftp->path_to_file)];
	char answer[MAX_STRING_SIZE];
	sprintf(command, "retr %s\n", ftp->path_to_file);

	printf("Requesting file %s\n",basename(ftp->path_to_file));

	if(ftp_send_command(ftp, command, 6+strlen(ftp->path_to_file)) < 0){
		printf("Error in requesting file from server\n");
		return -1;
	}
	if(ftp_read_answer(ftp, answer, MAX_STRING_SIZE) < 0){
		printf("Error in reading answer from file request from server\n");
		return -1;
	}

	int replyCode;
	sscanf(answer, "%d", &replyCode);
	if(replyCode == OPEN_FILE_FAIL){ /* Failed to find file */
		printf("Couldn't find file %s in server.\n", ftp->path_to_file);
		return -1;
	}
	else if(replyCode == OPEN_FILE_SUCCESS){
		/* Find the file size to export it */
		char* beg = strrchr(answer,'(');
		char sizeInfo[MAX_STRING_SIZE];
		strcpy(sizeInfo, beg);
		int size;
		sscanf(sizeInfo, "(%d bytes).", &size);
		ftp->file_size = size;
		#ifdef DEBUG
		printf("DEBUG : Size Info: %d bytes\n", size);
		#endif
	}

	#ifdef DEBUG
		printf("%s\n", answer);
	#endif

	return 0;
}

int ftp_disconnect(ftp_t* ftp){
	char answer[MAX_STRING_SIZE];
	if(ftp_read_answer(ftp, answer, MAX_STRING_SIZE) < 0){
		printf("Error in disconnecing from server\n");
		return -1;
	}

	if(ftp_send_command(ftp, "quit\n", strlen("quit\n")) < 0){
		printf("Error in sending quit command\n");
		return -1;
	}

	if(close(ftp->socketfd) < 0){
		printf("Failed to close ftp socket.\n");
		return -1;
	}

	if(close(ftp->datafd) < 0){
		printf("Failed to close ftp data socket.\n");
		return -1;
	}

	printf("Disconnected from server.\n");
	return 0;
}

void ftp_delete(ftp_t* ftp){
	free(ftp);
}