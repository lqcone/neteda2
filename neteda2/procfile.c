#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdint.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>

#include"procfile.h"
#include"log.h"


#define PF_PREFIX "PROCFILE"



procfile* procfile_open(const char *filename) {
	debug(D_PROCFILE,PF_PREFIX": Opening file is %s",filename);
	int fd = open(filename,O_RDONLY,0666);
	procfile* ff = malloc(sizeof(procfile));
	strncpy(ff->filename, filename, FILENAME_MAX);
	ff->filename[FILENAME_MAX] = "\0";

	ff->fd = fd;

	debug(D_PROCFILE, "File %s opened.", filename);
	return ff;


}