#include <regex.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#include "ftpClient.h"
#include "parseURL.h"


int validateURL(char* url, int size){
	/* TODO: Discutir com o Flavio se nao me enganei a fazer esta expressao regular */
	/* Notas: 	As passwords/usernames do sigarra apenas contem letras e numeros
				Fonte para os carateres autorizados num URL: http://stackoverflow.com/questions/7109143/what-characters-are-valid-in-a-url 
				Nao inclui o % no URL porque a trabalheira que ia ser parsar todos os url-encodes, mais valia nem usar regex */
	regex_t regex;
	char* regex_auth = "ftp://[[A-Za-z0-9]+:[A-Za-z0-9]+@][A-Za-z0-9._~:?#@!$&'()*+,:;=-]+/[A-Za-z0-9._~:/?#@!$&'()*+,:;=-]+";
	int reti = regcomp(&regex, regex_auth, REG_EXTENDED);
	char regcomp_err[255];

	/* Compile regex */
	reti = regcomp(&regex, regex_auth, REG_EXTENDED);
	if (reti) {
		regerror(reti, &regex, regcomp_err, sizeof(regcomp_err));
	    printf("Error while validating URL\n");
	    printf("Error while compiling regex: %s\n",regcomp_err);
	    return -1;
	}

	/* Execute regex */
	reti = regexec(&regex, url, 0, NULL, 0);

	/* Validate regex */
	if (!reti) {
	    #ifdef DEBUG
	    printf("DEBUG: Regex matched sucessfully\n");
	    #endif
	    return 0;
	}
	else if (reti == REG_NOMATCH) {
	    printf("Invalid URL!\n");
	    return 1;
	}
	else {
	    regerror(reti, &regex, regcomp_err, sizeof(regcomp_err));
	    printf("Error while validating URL: %s\n", regcomp_err);
	    printf("Error while validating URL: %s\n", regcomp_err);
	    return -1;
	}

}

void parseURL(char* url, int size, char* host, char* user, char* password, char* path){
	char* beg = url+7;
	#ifdef DEBUG
	printf("DEBUG: Url: %s\n",beg);
	#endif
	char* end =  strchr(beg, ':');
	int i = 0;
	while(beg != end){
		user[i] = *beg;
		++i;
		++beg;
	}
	++beg;
	user[i] = 0;

	end=strchr(beg,'@');
	i = 0;
	while(beg != end){
		password[i] = *beg;
		++i;
		++beg;
	}
	beg+=2;
	password[i] = 0;

	end = strchr(beg, '/');
	i = 0;
	while(beg != end){
		host[i] = *beg;
		++i;
		++beg;
	}
	++beg;
	host[i] = 0;

	strcpy(path, beg);

	#ifdef DEBUG
	printf("DEBUG: Host: %s, User: %s, Password: %s, Path: %s\n", host, user, password, path);
	#endif
}