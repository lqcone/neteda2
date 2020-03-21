#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdint.h>


#include"procfile.h"
#include"log.h"


#define PF_PREFIX "PROCFILE"



procfile* procfile_open(const char *filename,const char *separator,uint32_t flags) {
	debug(D_PROCFILE,PF_PREFIX": Opening file is %s",filename);

}