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

#define PFWORDS_INCREASE_STEP 200
#define PFLINES_INCREASE_STEP 10

#define PROCFILE_INCREMENT_BUFFER 512


pfwords* pfwords_new() {
	uint32_t size = PFWORDS_INCREASE_STEP;

	pfwords* new = malloc(sizeof(pfwords) + size * sizeof(char*));
	if (!new) return NULL;

	new->len = 0;
	new->size = size;
	return new;

}

void pfwords_reset(pfwords* fw) {
	fw->len = 0;
}

void pfwords_free(pfwords* fw) {
	
	free(fw);
}


pflines* pflines_new() {
	uint32_t size = PFLINES_INCREASE_STEP;

	pfwords* new = malloc(sizeof(pflines) + size * sizeof(char*));
	if (!new) return NULL;

	new->len = 0;
	new->size = size;
	return new;

}

void pflines_reset(pflines* fl) {
	fl->len = 0;
}

void pflines_free(pflines *fl){

	free(fl);
}


void procfile_close(procfile* ff) {
	debug(D_PROCFILE, PF_PREFIX, ": Closing file %s .",ff->filename);
	if (ff->lines) pflines_free(ff->lines);
	if (ff->words) pfwords_free(ff->words);
	if (ff->fd != -1) close(ff->fd);
	free(ff);

}


procfile* procfile_readall(procfile* ff) {
	debug(D_PROCFILE, PF_PREFIX, ":Reading file %s.", ff->filename);

	ssize_t t, s, r = 1, x;
	ff->len = 0;

	while (r > 0) {
		s = ff->len;
		x = ff->size - s;
		if(!x){
			debug(D_PROCFILE, PF_PREFIX, ": Expanding  data buffer for file %s.", ff->filename);

			procfile* new = realloc(ff, sizeof(procfile) + ff->size + PROCFILE_INCREMENT_BUFFER);
			if (!new) {
				error(PF_PREFIX, ": Cannot allocate memory for file %s", ff->filename);
				procfile_close(ff);
				return NULL;				
			}
			ff = new;
			ff->size += PROCFILE_INCREMENT_BUFFER;
		}
		debug(D_PROCFILE, "Reading file %s from position ld% with length %ld", ff->filename, s, ff->size - s);
		r = read(ff->fd, &ff->data[s], ff->size - s);
		if (r == -1) {
			procfile_close(ff);
			return NULL;
		}
		ff->len += r;

	}
	debug(D_PROCFILE, "Rewinding file %s", ff->filename);
	if (lseek(ff->fd, 0, SEEK_SET) == -1) {
		procfile_close(ff);
		return NULL;
	}
	pflines_reset(ff->lines);
	pfwords_reset(ff->words);




	debug(D_PROCFILE, "File %s updated", ff->filename);
	return ff;

}



procfile* procfile_open(const char *filename) {
	debug(D_PROCFILE,PF_PREFIX": Opening file is %s",filename);
	int fd = open(filename,O_RDONLY,0666);
	procfile* ff = malloc(sizeof(procfile));
	strncpy(ff->filename, filename, FILENAME_MAX);
	ff->filename[FILENAME_MAX] = "\0";

	ff->fd = fd;

	ff->lines = pflines_new();
	ff->words = pfwords_new();

	debug(D_PROCFILE, "File %s opened.", filename);
	return ff;
}