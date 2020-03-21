#include<sys/time.h>


#include"common.h"
#include"log.h"
#include"plugin_proc.h"
#include"rrd.h"


unsigned long long sutime(){
	struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000000ULL + now.tv_usec;
}

void* proc_main(void* ptr){
	
	//if (ptr) { ; }
	
	int vdo_proc_stat = 1;
	unsigned long long sunow;

	unsigned long long sutime_proc_stat = 0ULL;

	
	info("PROC Plugin thread created with task id %d", gettid());
	if (!vdo_proc_stat) {
		debug(D_PROCNETDEV_LOOP, "PROCNETDEV: calling_do_proc_stat");

	}

	for (; 1;) {
		debug(D_PROCNETDEV_LOOP, "PROCNETDEV: calling do proc_stat()");
		sunow = sutime(); 
		vdo_proc_stat = do_proc_stat(rrd_update_every, (sutime_proc_stat > 0) ? sunow - sutime_proc_stat : 0ULL);
		sutime_proc_stat = sunow;
	}

}