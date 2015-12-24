#include <regex.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#include "ftpClient.h"
#include "parseURL.h"

int validateURLAnon(char* url, int size);

int validateURLAnon(char* url, int size){
	regex_t regex;
	char* regex_auth_anon = "ftp://[A-Za-z0-9._~:?#!$&'()*+,:;=-]+/[A-Za-z0-9._~:/?#@!$&'()*+,:;=-]+";
	char regcomp_err[255];

	/* Compile regex */
	int reti = regcomp(&regex, regex_auth_anon, REG_EXTENDED);
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
	    printf("DEBUG: Anon regex matched sucessfully\n");
	    #endif
	    return 1;
	}
	else if (reti == REG_NOMATCH) {
	    printf("Invalid URL!\n");
	    return 2;
	}
	else {
	    regerror(reti, &regex, regcomp_err, sizeof(regcomp_err));
	    printf("Error while validating URL: %s\n", regcomp_err);
	   	#ifdef DEBUG
	    printf("DEBUG: Error while matching regex: %s\n", regcomp_err);
	    #endif
	    return -1;
	}
}

int validateURL(char* url, int size){
	regex_t regex;
	char* regex_auth = "ftp://[A-Za-z0-9]+:[A-Za-z0-9]+@[A-Za-z0-9._~:?#!$&'()*+,:;=-]+/[A-Za-z0-9._~:/?#@!$&'()*+,:;=-]+";
	char regcomp_err[255];

	/* Compile regex */
	int reti = regcomp(&regex, regex_auth, REG_EXTENDED);
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
	    return validateURLAnon(url, size);
	}
	else {
	    regerror(reti, &regex, regcomp_err, sizeof(regcomp_err));
	    printf("Error while validating URL: %s\n", regcomp_err);
	   	#ifdef DEBUG
	    printf("DEBUG: Error while matching regex: %s\n", regcomp_err);
	    #endif
	    return -1;
	}

}

void parseURL(char* url, int size, char* host, char* user, char* password, char* path, int anon){
	// Remove ftp:// from string
	char* beg = url+6;
	#ifdef DEBUG
	printf("DEBUG: Url: %s\n",beg);
	#endif
	if(anon == 0) {
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
		++beg;
		password[i] = 0;
	}

	else {
		strncpy(user, "anonymous", sizeof("anonymous"));
		printf("Please specify your email address for authentication: ");
		scanf("%s", password);
	}

	char* end = strchr(beg, '/');
	int i = 0;
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