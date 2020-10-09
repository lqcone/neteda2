#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>
#include<time.h>
#include<sys/select.h>

#include"log.h"
#include"web_client.h"


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


	struct web_client* w;
	struct timeval tv;
	int retval;

	info("WEB SERVER thread created with task id %d", gettid());

	if (listen_fd < 0) fatal("LISTENER: Listen socket is not ready.");

	fd_set ifds, ofds, efds;
	int fdmax = listen_fd;

	FD_ZERO(&ifds);
	FD_ZERO(&ofds);
	FD_ZERO(&efds);

	for (;;) {

		tv.tv_sec = 0;
		tv.tv_usec = 200000;

		if (listen_fd >= 0) {
			FD_SET(listen_fd, &ifds);
			FD_SET(listen_fd, &efds);
		}

		retval = select(fdmax + 1, &ifds, &ofds, &efds, &tv);

		if (retval == -1) {
			error("LISTENER: select() failed");
			continue;
		}

		else if (retval) {
			//check for the incoming connections
			if (FD_ISSET(listen_fd, &ifds)) {
				w = web_client_create(listen_fd);
				if (!w) continue;

				if (pthread_create(&w->pthread, NULL, web_client_main, w) != 0) {
					error("%llu:failed to create new thread for web client.");
					w->obsolete = 1;
				}

				//cleanup unused client
				
			}
		}

		for (w = web_clients; w; w = w ? w->next : NULL) {
			//info("checking web client %d", w->id);
			if (w->obsolete) { 
				info("free web client %d", w->id);
				w = web_client_free(w);
			}
		}

	}


}