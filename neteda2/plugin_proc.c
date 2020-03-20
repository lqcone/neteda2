#include"common.h"
#include"log.h"


void* proc_main(void* ptr){
	
	int vdo_proc_stat = 1;

	
	info("PROC Plugin thread created with task id %d", gettid());
	if (!vdo_proc_stat) {
		debug(D_PROCNETDEV_LOOP, "PROCNETDEV: calling_do_proc_stat");

	}

}