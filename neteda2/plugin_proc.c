#include"common.h"
#include"log.h"


void* proc_main(void* ptr){
	if (ptr) { ; }       //�̱߳���е�һ�ֻ��ƣ�

	info("PROC Plugin thread created with task id %d", gettid());

}