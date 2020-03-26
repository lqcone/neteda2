#include<stdio.h>
#include<stdlib.h>
#include"procfile.h"
#include"common.h"
int do_proc_stat(int update_every,unsigned long long dt){
	static procfile* ff = NULL;
	static int do_cpu = 1;
	if (!ff) {
		char filename[FILENAME_MAX + 1];
		snprintf(filename,FILENAME_MAX,"%s%s",global_host_prefix,"/proc/stat" );
		ff = procfile_open("/proc/stat");
		if (!ff) return 1;

	}
	return 0;
}	