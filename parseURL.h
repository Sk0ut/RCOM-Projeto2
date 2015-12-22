#ifndef __parseURL
#define __parseURL

void parseURL(char* url, int size, char* host, char* user, char* password, char* path, int anon);
int validateURLAnon(char* url, int size);
int validateURL(char* url, int size);

#endif 