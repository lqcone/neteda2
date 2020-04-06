#include<sys/types.h>
#include<syscall.h>

#include"strsep.h"



char* global_host_prefix = "";


char* mystrsep(char** ptr, char* s) {
	char* p = "";
	while (p && !p[0] && *ptr) p = strsep_lqc(ptr, s);
	return p;
}


pid_t gettid(void){
	return syscall(SYS_gettid);
}