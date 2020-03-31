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


pfwords* pfwords_add(pfwords* fw, char* str) {
	if (fw->len == fw->size) {
		pfwords* new = realloc(fw, sizeof(pfwords) + (fw->size + PFWORDS_INCREASE_STEP) * sizeof(char*));
		if (!new) {
			error(PF_PREFIX": failed to expand words");
			free(fw);
			return NULL;
		}
		fw = new;
		fw->size += PFWORDS_INCREASE_STEP;
	}
	fw->words[fw->len++] = str;
	return fw;
}
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


pflines* pflines_add(pflines* fl, uint32_t first_word) {
	if (fl->len == fl->size) {
		pflines* new = realloc(fl, sizeof(pflines) + (fl->size + PFLINES_INCREASE_STEP) * sizeof(ffline));
		if (!new) {
			error(PF_PREFIX": failed to expand lines");
			free(fl);
			return NULL;
		}
		fl = new;
		fl->size += PFLINES_INCREASE_STEP;
	}
	fl->lines[fl->len].words = 0;
	fl->lines[fl->len++].first = first_word;

	return fl;
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



#define PF_CHAR_IS_SEPARATOR	' '
#define PF_CHAR_IS_NEWLINE		'N'
#define PF_CHAR_IS_WORD			'W'
#define PF_CHAR_IS_QUOTE        'Q'
#define PF_CHAR_IS_OPEN         'O'
#define PF_CHAR_IS_CLOSE        'C'

void procfile_close(procfile* ff) {
	debug(D_PROCFILE, PF_PREFIX, ": Closing file %s .",ff->filename);
	if (ff->lines) pflines_free(ff->lines);
	if (ff->words) pfwords_free(ff->words);
	if (ff->fd != -1) close(ff->fd);
	free(ff);

}

procfile* procfile_parser(procfile* ff) {
	debug(D_PROCFILE, PF_PREFIX": Parsing file %s", ff->filename);

	char* s = ff->data, * e = &ff->data[ff->len], * t = ff->data, quote = 0;
	uint32_t l = 0, w = 0;
	int opened = 0;

	ff->lines = pflines_add(ff->lines, w);
	if (!ff -> lines) goto cleanup;
	s = e;     
	if (s > t&&t<e) {
		if (ff->len < ff->size)
			*s = '\0';
		else {
			ff -> data[ff->size - 1] = '\0';
		}

		ff->words = pfwords_add(ff->words, t);
		if (!ff->words) goto cleanup;
		
		ff->lines->lines[l].words++;
		
	}
	
	return ff;

cleanup:
	error(PF_PREFIX ": Failed to parse file %s", ff->filename);
	procfile_close(ff);
	return NULL;
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

	ff = procfile_parser(ff);


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