#include <stdio.h>
#include <stdlib.h>

#include "ftpClient.h"
#include "parseURL.h"

#ifndef NULL
#define NULL (void*)0
#endif

void printUsage(){
	printf("Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
}

int main(int argc, char** argv){
	if(argc != 2){
		printUsage();
		return 1;
	}

	int val = validateURL(argv[1], sizeof(argv[1]));

	if(val != 0){
		if(val == 1)
			printUsage();
		return 1;
	}

	char host[MAX_STRING_SIZE];
	char user[MAX_STRING_SIZE];
	char password[MAX_STRING_SIZE];
	char path[MAX_STRING_SIZE];

	parseURL(argv[1], sizeof(argv[1]), host, user, password, path);

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