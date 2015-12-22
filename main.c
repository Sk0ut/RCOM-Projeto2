#include <stdio.h>
#include <stdlib.h>

#include "ftpClient.h"
#include "parseURL.h"

#ifndef NULL
#define NULL (void*)0
#endif

void printUsage(){
	printf("Usage: Normal: download ftp://<user>:<password>@<host>/<url-path>\n");
	printf("Usage: Anon: download ftp://<host>/<url-path>\n");

}

int main(int argc, char** argv){
	if(argc != 2){
		printUsage();
		return 1;
	}

	int val = validateURL(argv[1], sizeof(argv[1]));
	int anon = 1;
	char user[MAX_STRING_SIZE];
	char password[MAX_STRING_SIZE];
	char host[MAX_STRING_SIZE];
	char path[MAX_STRING_SIZE];
	switch(val){
		case 0:
			anon = 0;
		case 1:
			parseURL(argv[1], sizeof(argv[1]), host, user, password, path, anon);
			break;
		case 2:
			printUsage();
			return 1;
		default:
			return 1;
	}

	ftp_t* ftp = ftp_init(host, user, password, path);
	if(ftp == NULL)
		return 1;

	if(ftp_connect(ftp) < 0)
		return 1;

	if(ftp_login_host(ftp) < 0)
		return 1;

	if(ftp_set_passive_mode(ftp) < 0)
		return 1;

	if(ftp_retr_file(ftp) < 0)
		return 1;

	if(ftp_download_file(ftp) < 0)
		return 1;

	if(ftp_disconnect(ftp) < 0)
		return 1;

	ftp_delete(ftp);

	return 0;
}