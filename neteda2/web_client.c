#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<pthread.h>
#include<sys/select.h>
#include<sys/time.h>
#include<string.h>

#include"web_client.h"
#include"log.h"
#include"web_buffer.h"
#include"strsep.h"
#include"url.h"
#include"common.h"


#define INITAL_WEB_DATA_LENGTH 16384

int web_client_timeout = DEFAULT_DISCONNECT_IDLE_WEB_CLIENTS_AFTER_SECOND;


struct web_client* web_clients = NULL;
unsigned long long web_clients_count = 0;


struct web_client* web_client_create(int listener) {
	struct web_client* w;
	info("web_client_create started.");
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



	}

	w->response.data = buffer_create(INITAL_WEB_DATA_LENGTH);
	if (!w->response.data) {
		close(w->ifd);
		free(w);
		return NULL;
	}



	w->wait_receive = 1;
	return w;
}


struct web_client* web_client_free(struct web_client* w) {


	return w;
}


void web_client_process(struct web_client *w){
	

	if (strstr(w->response.data->buffer, "\r\n\r\n")) {
		char* buf = (char*)buffer_tostring(w->response.data);
		char* url = NULL;
		
		char *tok = strsep_lqc(&buf, " \r\n");
		if (buf&&strcmp(tok, "GET") == 0) {
			tok = strsep_lqc(&buf, " \r\n");
			url = url_decode(tok);
		}

		w->last_url[0] = '\0';

		if (url) {
			strncpy(w->last_url, url, URL_MAX);
			w->last_url[URL_MAX] = '\0';
			tok = mystrsep(&url, "/?");
			if (tok && *tok) {
				info("tok and *tok is not NULL.");
			}
			else {
				char filename[FILENAME_MAX + 1];
				url = filename;
				strncpy(filename, w->last_url, FILENAME_MAX);
				filename[FILENAME_MAX] = '\0';
				tok = mystrsep(&url, "?");
				buffer_flush(w->response.data);
				code=
			}
		}

	}

}


long web_client_receive(struct web_client* w) {
	
	long left = w->response.data->size - w->response.data->len;
	long bytes;

	bytes = recv(w->ifd, &w->response.data->buffer[w->response.data->len], (size_t)(left - 1), MSG_DONTWAIT);
	if (bytes > 0) {
		w->response.data->len += bytes;
		w->response.data->buffer[w->response.data->len] = '\0';
	}

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

	//info("web_client_main started");
	
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

		info("waiting for input data with select().");

		retval = select(fdmax + 1, &ifds, &ofds, &efds,&tv);

		info("func select() end");

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

	//info("web_client_main end.");


	return NULL;

}
