#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#include"log.h"


int listen_fd = -1;

int create_listen_socket4() {
	
	int sock;

	return sock;
}


void* socket_listen_main(void* ptr) {

	info("WEB SERVER thread created with task id %d", gettid());

}