#include"config.h"
#ifdef NETDATA_WITH_ZLIB
#include <zlib.h>
#endif

#include"web_buffer.h"




#define DEFAULT_DISCONNECT_IDLE_WEB_CLIENTS_AFTER_SECOND 10

#ifndef NETDATA_WEB_CLIENT_H
#define NETDATA_WEB_CLIENT_H 1

#define WEB_CLIENT_MODE_NORMAL      0
#define	WEB_CLIENT_MODE_FILECOPY    1
#define WEB_CLIENT_MODE_OPTIONS		2

#define URL_MAX 8192
#define ZLIB_CHUNK 	16384
#define HTTP_RESPONSE_HEADER_SIZE 4096


struct response {
	BUFFER* header;
	BUFFER* header_output;
	BUFFER* data;

	int code;

	size_t rlen;              //如果设置为非0，表示ifd期待的大小
	size_t sent;              //当前已发送数据长度
	int zoutput;

#ifdef NETDATA_WITH_ZLIB
	z_stream zstream;				// zlib stream for sending compressed output to client
	Bytef zbuffer[ZLIB_CHUNK];		// temporary buffer for storing compressed output
	long zsent;						// the compressed bytes we have sent to the client
	long zhave;						// the compressed bytes that we have to send
	int zinitialized;
#endif
};



struct web_client{

	unsigned long long id;

	char last_url[URL_MAX + 1];

	int mode;
	int keepalive;

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