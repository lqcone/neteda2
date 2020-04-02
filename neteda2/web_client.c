#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<pthread.h>
#include<sys/select.h>
#include<sys/time.h>

#include"web_client.h"
#include"log.h"


int web_client_timeout = DEFAULT_DISCONNECT_IDLE_WEB_CLIENTS_AFTER_SECOND;


struct web_client* web_clients = NULL;
unsigned long long web_clients_count = 0;


struct web_client* web_client_create(int listener) {
	struct web_client* w;

	w = calloc(1, sizeof(struct web_client));
	if (!w) {
		error("Cannot allocate new web_client memory .");
		return NULL;
	}

	w->id = ++web_clients_count;

	{
		struct sockaddr* sadr;
		socklen_t addrlen;

		sadr = (struct sockaddr*) & w->clientaddr;
		addrlen = sizeof(w->clientaddr);

		w->ifd = accept(listener, sadr, &addrlen);
		if (w->ifd == -1) {
			error("%llu: Cannot accept  newo incoming connection.",w->id);
			free(w);
			return NULL;
		}


		if (web_clients) web_clients->prev = w;
		w->next = web_clients;
		web_clients = w;

		return w;

	}


	w->wait_receive = 1;
	return w;
}


struct web_client* web_client_free(struct web_client* w) {


	return w;
}


void web_client_process(struct web_client *w){

}


long web_client_receive(struct web_client* w) {
	long bytes;


		return bytes;
}

long web_client_send(struct web_client* w) {

	long bytes;



	return bytes;

}

void* web_client_main(void* ptr) {

	struct timeval tv;
	struct web_client* w = ptr;
	int retval;

	fd_set ifds, ofds, efds;
	int fdmax = 0;

	for (;;) {
		FD_ZERO(&ifds);
		FD_ZERO(&ofds);
		FD_ZERO(&efds);

		FD_SET(w->ifd, &efds);

		if (w->wait_receive) {
			FD_SET(w->ifd, &ifds);
			if (w->ifd > fdmax) fdmax = w->ifd;
		}
		tv.tv_sec = web_client_timeout;
		tv.tv_usec = 0;

		retval = select(fdmax + 1, &ifds, &ofds, &efds,&tv);

		if (retval == -1) { continue; }
		
		else if (!retval) { break; }

		if (FD_ISSET(w->ifd, &efds)) {

			break;
		}
		if (w->wait_receive && FD_ISSET(w->ifd, &ifds)) {
			long bytes;
			if (bytes = web_client_receive(w) < 0) {

				break;
			}
			web_client_process(w);
		}
	}
	
	w->obsolete = 1;
	return NULL;

}
