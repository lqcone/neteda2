#include<sys/types.h>
#include<stdint.h>

#ifndef NETDATA_COMMON_H
#define NETDATA_COMMON_H 1

#define CACHE_DIR              "/home/nick/projects/netdata_bin/var/cache/netdata"
#define LOG_DIR                "/home/nick/projects/netdata_bin/var/log/netdata"
#define CONFIG_DIR               "/home/nick/projects/netdata_bin/etc/netdata"
#define RUN_DIR                "/home/nick/projects/netdata_bin/var/run"


extern char *global_host_prefix;

extern uint32_t simple_hash(const char* name);

extern char* mystrsep(char** ptr, char* s);
extern char* trim(char* s);


extern pid_t gettid(void);



#endif