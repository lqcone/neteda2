#include"web_buffer.h"




#define DEFAULT_DISCONNECT_IDLE_WEB_CLIENTS_AFTER_SECOND 60

#ifndef NETDATA_WEB_CLIENT_H
#define NETDATA_WEB_CLIENT_H 1


#define URL_MAX 8192


struct response {
	BUFFER* header;
	BUFFER* header_output;
	BUFFER* data;
};



struct web_client{

	unsigned long long id;

	char last_url[URL_MAX + 1];

	struct sockaddr_storage clientaddr;

	pthread_t pthread;
	int obsolete;           //if set to 1,the listener will remove this client

	int ifd;
	int ofd;

	struct response response;

	int wait_receive;
	int wait_send;

	struct web_client* prev;
	struct web_client* next;

};


extern struct web_client* web_clients;

extern struct web_client* web_client_create(int listener);
extern struct web_client* web_client_free(struct web_client* w);


extern void* web_client_main(void* ptr);


#endif