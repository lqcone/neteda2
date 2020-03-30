#include<sys/types.h>
#include<syscall.h>




char* global_host_prefix = "";

pid_t gettid(void){
	return syscall(SYS_gettid);
}