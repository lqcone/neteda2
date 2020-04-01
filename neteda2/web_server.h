
#define LISTEN_PORT 19999

extern int listen_fd;

extern int create_listen_socket4();

extern void* socket_listen_main(void* ptr);