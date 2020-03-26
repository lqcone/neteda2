#include<sys/types.h>
#include<syscall.h>





pid_t gettid(void){
	return syscall(SYS_gettid);
}