#include<sys/types.h>
#include<stdint.h>

#ifndef NETDATA_COMMON_H
#define NETDATA_COMMON_H 1


extern char *global_host_prefix;

extern uint32_t simple_hash(const char* name);

extern char* mystrsep(char** ptr, char* s);
extern char* trim(char* s);


extern pid_t gettid(void);



#endif