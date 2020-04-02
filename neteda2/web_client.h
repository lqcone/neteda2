

#define DEFAULT_DISCONNECT_IDLE_WEB_CLIENTS_AFTER_SECOND 60

struct web_client{

	unsigned long long id;

	struct sockaddr_storage clientaddr;

	pthread_t pthread;
	int obsolete;           //if set to 1,the listener will remove this client

	int ifd;
	int ofd;

	int wait_receive;
	int wait_send;

	struct web_client* prev;
	struct web_client* next;

};


extern struct web_client* web_clients;

extern struct web_client* web_client_create(int listener);
extern struct web_client* web_client_free(struct web_client* w);


extern void* web_client_main(void* ptr);