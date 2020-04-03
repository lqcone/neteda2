#include<stdio.h>
#include<stdlib.h>

#include"web_buffer.h"
#include"log.h"

BUFFER* buffer_create(long size) {

	BUFFER* b;

	b = calloc(1, sizeof(BUFFER));
	if (!b) {
		error("Cannot allocate a web buffer. ");
		return NULL;
	}
	b->buffer = malloc(size);
	if (!b->buffer) {
		error("Cannot allocate a buffer of size%u", size);
		free(b);
		return NULL;
	}
	b->buffer[0] = '\0';
	b->size = size;


	return b;
}



