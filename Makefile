all:
	gcc -Wall -o download main.c ftpClient.c parseURL.c

clean:
	rm download
