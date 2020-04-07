#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<pthread.h>
#include<sys/select.h>
#include<sys/time.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<errno.h>

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
	w->mode = WEB_CLIENT_MODE_NORMAL;

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
		w->ofd = w->ifd;        //输入输出通用同一套接字


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

int mysendfile(struct web_client* w, char* filename) {
	static char* web_dir = NULL;
	if (!web_dir) web_dir = "/home/nick/projects/neteda2";

	while (*filename == '/') filename++;

	char webfilename[FILENAME_MAX + 1];
	snprintf(webfilename, FILENAME_MAX, "%s/%s", web_dir, filename);
	struct stat stat;
	if (lstat(webfilename, &stat) != 0) {
		return 404;
	}

	if ((stat.st_mode & S_IFMT) == S_IFDIR) {
		snprintf(webfilename, FILENAME_MAX + 1, "%s/index.html", filename);
		return mysendfile(w, webfilename);
	}

	//open the file
	w->ifd = open(webfilename, O_NONBLOCK, O_RDONLY);
	if (w->ifd == -1) {
		w->ifd = w->ofd;
		if (errno == EBUSY || errno == EAGAIN) {
			return 307;
		}

		else {
			return 404;
		}
	}
	w->mode = WEB_CLIENT_MODE_FILECOPY;
	w->wait_receive = 1;
	w->wait_send = 0;
	buffer_flush(w->response.data);
	w->response.rlen = stat.st_size;


	return 200;
}


void web_client_process(struct web_client *w){
	
	int code = 500;
	ssize_t bytes;

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
				code = mysendfile(w, (tok && *tok) ? tok : "/");
			}
		}

	}


}


long web_client_send(struct web_client* w) {
	long bytes;

	bytes = send(w->ofd, &w->response.data->buffer[w->response.sent], w->response.data->len - w->response.sent, MSG_DONTWAIT);
	if (bytes > 0) {

		w->response.sent += bytes;
	}

	return bytes;
}

long web_client_receive(struct web_client* w) {
	
	long left = w->response.data->size - w->response.data->len;
	long bytes;
	if (w->mode == WEB_CLIENT_MODE_FILECOPY)
		bytes = read(w->ifd, &w->response.data->buffer[w->response.data->len], (ssize_t)(left - 1));
	else
		bytes = recv(w->ifd, &w->response.data->buffer[w->response.data->len], (size_t)(left - 1), MSG_DONTWAIT);
	if (bytes > 0) {
		w->response.data->len += bytes;
		w->response.data->buffer[w->response.data->len] = '\0';
		
		if (w->mode == WEB_CLIENT_MODE_FILECOPY) {
			w->wait_send = 1;
			if (w->response.rlen && w->response.data->len >= w->response.rlen)
				w->wait_receive = 0;

		}
	}
	else if (bytes == 0) {

		if (w->mode = WEB_CLIENT_MODE_FILECOPY) {
			
			w->wait_receive = 0;
		}
	}

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

		if (w->ifd != w->ofd)
			FD_SET(w->ofd, &efds);

		if (w->wait_receive) {
			FD_SET(w->ifd, &ifds);
			if (w->ifd > fdmax) fdmax = w->ifd;
		}

		if (w->wait_send) {
			FD_SET(w->ofd, &ofds);
			if (w->ofd > fdmax) fdmax = w->ofd;
		}

		tv.tv_sec = web_client_timeout;
		tv.tv_usec = 0;


		retval = select(fdmax + 1, &ifds, &ofds, &efds,&tv);


		if (retval == -1) { continue; }
		
		else if (!retval) { break; }

		if (FD_ISSET(w->ifd, &efds)) {

			break;
		}
		if (w->wait_send && FD_ISSET(w->ofd, &ofds)) {
			long bytes;
			if (bytes = web_client_send(w) < 0) {

				break;
			}
		}
		if (w->wait_receive && FD_ISSET(w->ifd, &ifds)) {
			long bytes;
			if (bytes = web_client_receive(w) < 0) {

				break;
			}
			if (w->mode == WEB_CLIENT_MODE_NORMAL) {
				web_client_process(w);
			}
		}
	}
	
	w->obsolete = 1;

	//info("web_client_main end.");


	return NULL;

}
