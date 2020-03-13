#include"common.h"
#include"log.h"


void* proc_main(void* ptr){
	if (ptr) { ; }       //线程变成中的一种机制？

	info("PROC Plugin thread created with task id %d", gettid());

}