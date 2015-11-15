#ifndef __tcpClient
#define __tcpClient

#define DEBUG
#define FTP_PORT 21
#define MAX_STRING_SIZE	255
#define IP_MAX_SIZE	15

typedef struct {
	char server_address[IP_MAX_SIZE]; //NNN.NNN.NNN.NNN
	int socketfd;
	int datafd;
	char username[MAX_STRING_SIZE];
	char password[MAX_STRING_SIZE];
	char path_to_file[MAX_STRING_SIZE];
} ftp_t;

int getIpAdress(const char* server_addr, char* buf);
ftp_t* ftp_init(const char* server, const char* username, const char* password, const char* path_to_file);
int ftp_connect_socket(const char* ip, const int port);
int ftp_connect(ftp_t* ftp);
int ftp_send_command(ftp_t* ftp, const char* command, const int size);
int ftp_read_answer(ftp_t* ftp, char* answer, const int size);
int ftp_login_host(ftp_t* ftp);
int ftp_set_passive_mode(ftp_t* ftp);
int ftp_download_file(ftp_t* ftp);
int ftp_retr_file(ftp_t* ftp);
int ftp_disconnect(ftp_t* ftp);
void ftp_delete(ftp_t* ftp);

#endif