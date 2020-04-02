

#define LISTEN_BACKLOG 100
#define LISTEN_PORT 19999

extern int listen_fd;

extern int create_listen_socket4(int port,int listen_baklog);

extern void* socket_listen_main(void* ptr);