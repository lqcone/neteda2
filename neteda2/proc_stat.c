#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"procfile.h"
#include"common.h"
#include"rrd.h"


#define RRD_TYPE_STAT     "cpu"

int do_proc_stat(int update_every,unsigned long long dt){
	static procfile* ff = NULL;
	static int do_cpu = 1,   do_cpu_cores = -1;
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
	RRDSET* st;

	for (l = 0; l < lines; l++) {
		if (strncmp(procfile_lineword(ff, l, 0), "cpu", 3) == 0) {
			words = procfile_linewords(ff, l);
			if (words < 9) {
				error("Cannot read /proc/stat cpu line. Expected 9 params, read %d", words);
				continue;
			}

			char* id;
			unsigned long long user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guest_nice = 0;

			id = procfile_lineword(ff, l, 0);
			user = strtoull(procfile_lineword(ff, l, 1), NULL, 10);
			nice = strtoull(procfile_lineword(ff, l, 2), NULL, 10);
			system = strtoull(procfile_lineword(ff, l, 3), NULL, 10);
			idle = strtoull(procfile_lineword(ff, l, 4), NULL, 10);
			iowait = strtoull(procfile_lineword(ff, l, 5), NULL, 10);
			irq = strtoull(procfile_lineword(ff, l, 6), NULL, 10);
			softirq = strtoull(procfile_lineword(ff, l, 7), NULL, 10);
			steal = strtoull(procfile_lineword(ff, l, 8), NULL, 10);
			if (words >= 10) guest = strtoull(procfile_lineword(ff, l, 9), NULL, 10);
			if (words >= 11) guest_nice = strtoull(procfile_lineword(ff, l, 10), NULL, 10);


			char* title = "Core utilization";
			char* type = RRD_TYPE_STAT;
			char* context = "cpu.cpu";
			char* family = "utilization";
			long priority = 1000;
			int isthistotal = 0;

			if (strcmp(id, "cpu") == 0) {
				isthistotal = 1;
				type = "system";
				title = "Total CPU utilization";
				context = "system.cpu";
				family = id;
				priority = 100;
			}
			if ((isthistotal && do_cpu)||(!isthistotal&&do_cpu_cores)) {
				st = rrdset_find_bytype(type, id);
				if (!st) {
					st = rrdset_create(type, id, NULL, family, context, title, "percentage", priority, update_every, RRDSET_TYPE_STACKED);
				}
			}


		}
	}

	return 0;
}	