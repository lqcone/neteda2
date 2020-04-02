#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>

#include"log.h"


int listen_fd = -1;

int create_listen_socket4(int port,int listen_backlog) {
	
	int sock;
	struct sockaddr_in name;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		error("IPv4 socket() failed");
		return -1;
	}

	memset(&name, 0, sizeof(struct sockaddr_in));
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	info("Listening on any IPs(IPv4.)");

	if (bind(sock, (struct sockaddr*) & name, sizeof(name)) < 0) {
		close(sock);
		error("IPv4 bind() failed.");
		return -1;
	}

	if (listen(sock, listen_backlog) < 0) {
		close(sock);
		fatal("IPv4 listen() failed.");
		return - 1;
	}



	return sock;
}


void* socket_listen_main(void* ptr) {

	info("WEB SERVER thread created with task id %d", gettid());

	if (listen_fd < 0) fatal("LISTENER: Listen socket is not ready.");


}