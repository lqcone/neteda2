#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"procfile.h"
#include"common.h"
int do_proc_stat(int update_every,unsigned long long dt){
	static procfile* ff = NULL;
	static int do_cpu = 1;
	if (!ff) {
		char filename[FILENAME_MAX + 1];
		snprintf(filename,FILENAME_MAX,"%s%s",global_host_prefix,"/proc/stat" );
		ff = procfile_open("/proc/stat","\t:");
	}
	if (!ff) return 1;

	ff = procfile_readall(ff);
	if (!ff) return 0;     //这里我们返回0，以便于下次重新尝试打开文件

	uint32_t lines = procfile_lines(ff), l;
	uint32_t words;

	for (l = 0; l < lines; l++) {
		if (strncmp(procfile_lineword(ff, l, 0), "cpu", 3) == 0) {
			words = procfile_linewords(ff, l);
		}
	}

	return 0;
}	