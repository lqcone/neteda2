
#include<syslog.h>
#include<pthread.h>
#include<stdlib.h>
#include "log.h"
#include"plugin_proc.h"




struct netdata_static_thread {
	char* name;

	char* config_section;
	char* config_name;

	int enabled;

	pthread_t* thread;

	void (*init_routine) (void);
	void* (*start_routine) (void*);
};

struct netdata_static_thread static_threads[] = {
	{"proc",		"plugins",	"proc",			1, NULL, NULL,	proc_main}
};



void main()
{
	int i;
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
	


    
}