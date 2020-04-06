#include<sys/types.h>

#ifndef NETDATA_COMMON_H
#define NETDATA_COMMON_H 1


extern char *global_host_prefix;

extern char* mystrsep(char** ptr, char* s);

extern pid_t gettid(void);



#endif