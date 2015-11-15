#ifndef __parseURL
#define __parseURL

void parseURL(char* url, int size, char* host, char* user, char* password, char* path);
int validateURL(char* url, int size);

#endif 