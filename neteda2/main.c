
#include<syslog.h>
#include<pthread.h>
#include<stdlib.h>
#include "log.h"
#include"plugin_proc.h"
#include"web_server.h"




int netdata_exit = 1;

struct netdata_static_thread {
	char *name;

	char *config_section;
	char *config_name;

	int enabled;

	pthread_t* thread;

	void (*init_routine) (void);
	void *(*start_routine) (void*);
};

struct netdata_static_thread static_threads[] = {
	{"proc",		"plugins",	"proc",			1, NULL, NULL,	proc_main},

    {"web",          NULL,       NULL,          1, NULL, NULL,socket_listen_main},
	{NULL,			 NULL,		 NULL,			0, NULL, NULL,  NULL}          //地址占用，确保不能访问其他变量地址
};



void main()
{
	int i;


	{
		if (listen_fd < 0) {
			listen_fd = create_listen_socket4();
			


		}
	}


	for (i = 0; static_threads[i].name != NULL; i++) {
		struct netdata_static_thread* st = &static_threads[i];
		if (st->enabled) {
			st->thread = malloc(sizeof(pthread_t));
			if (!st->thread)
				fatal("Can not allocate pthread_t memory");

			info("Starting thread %s.", st->name);

			if (pthread_create(st->thread, NULL, st->start_routine, NULL))
				error("faild to create new thread for %s", st->name);
			else if (pthread_detach(*st->thread))
				error("Can not request detach of newly created %s thread.", st->name);

		}
		else info("Not starting thread %s.", st->name);
	}

	for (;;) {
		sleep(1);
	}

    
}