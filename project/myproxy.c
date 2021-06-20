#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUF_SIZE 2048

void error(char *msg) {
	perror(msg);
	exit(1);
}

void open_clientfd(char *hostname, int port) {
	return;
}

void read_header(int fd, void *ptr, size_t bytes) {
	return;
}

void write_header(int fd, void *buf, size_t bytes) {
	return;
}

void proxy_handler(int cli_sockfd) {
	char proxy_header[BUF_SIZE];
	char buffer[BUF_SIZE];
	int serverfd, clientfd, n;
	
	recv(cli_sockfd, buffer, BUF_SIZE, 0);

	printf("[Proxy Request]\n%s\n", buffer);

	char method[100];
	char filename[500];
	char rest[1000];

	sscanf(buffer, "%s %s %s", method, filename, rest);

	printf("[Filename]: %s\n", filename);

	char *host = strstr(filename, "www");
	char *temp_host = malloc(strlen(host) + 1);
	char *temp_host2 = malloc(strlen(host) + 1);
	strcpy(temp_host, host);
	strcpy(temp_host2, host);
	printf("The host string is initially %s\n", host);

	char *f = index(host, ":");
	if (f == NULL) {
		host = strtok(host, "/");
	} else {
		host = strtok(host, ":");
	}

	char *tmp = NULL;
	tmp = strtok(temp_host, "/");
	tmp = strtok(temp_host, ":");

	if (tmp == NULL) {
		if ((serverfd = open_clientfd(host, 80)) < 0) {
			error("Open Server");
			exit(1);
		} else {
			int tok;
			++tmp;
			tok = atoi(tmp);
			if ((serverfd = open_clientfd(host, tok)) < 0) {
				error("Open Server");
				exit(1);
			}
		}
	}

	write_header(serverfd, "GET ", strlen("GET "));
	write_header(serverfd, filename, strlen(filename));
	write_header(serverfd, " HTTP/1.0\r\n\r\n", strlen(" HTTP/1.0\r\n\r\n"));

	int response_len = 0;

	while ((n = read_header(serverfd, buffer, BUF_SIZE)) > 0) {
		response_len += n;
		write_header(clientfd, buffer, n);
		bzero(buffer, BUF_SIZE);
	}

	close(clientfd);
	close(serverfd);
}

int main(int argc, char *argv[]) {
	int portno, pid;
	int serv_sockfd, cli_sockfd;

	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	char request_buffer[BUF_SIZE];
	rio_t rio;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[1]);
	printf("The proxy server will listen to port: %d\n", portno);

	serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_sockfd < 0) {
		error("ERROR: Failed to create socket.\n");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	
	if (bind(serv_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR: Binding\n");
		exit(1);
	}
	
	if (listen(serv_sockfd, 5) < 0) {
		error("ERROR: Listening\n");
		exit(1);
	}

	signal(SIGCHLD, SIG_IGN);

	while (1) {
		printf("Waiting Connections...\n");
		
		clilen = sizeof(cli_addr);
		cli_sockfd = accept(serv_sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (cli_sockfd < 0) {
			error("ERROR: Accepting\n");
			continue;
		}

		pid = fork();
		if (pid == 0) {
			close(serv_sockfd);
			proxy_handler(cli_sockfd);
			close(cli_sockfd);
			exit(0);
		} else if (pid > 0) {
			close(cli_sockfd);
		} else {
			error("ERROR: Forking Process\n");
			exit(0);
		}
	}
}


