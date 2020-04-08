#include<stdio.h>
#include<stdlib.h>

#include"web_buffer.h"
#include"log.h"


const char* buffer_tostring(BUFFER* wb) {
	
	wb->buffer[wb->len] = '\0';
	return(wb->buffer);
}

void buffer_sprintf(BUFFER* wb, const char* fmt, ...)
{
	if (!fmt || !*fmt) return;

	buffer_need_bytes(wb, 1);

	size_t len = wb->size - wb->len, wrote;

	va_list args;
	va_start(args, fmt);
	wrote = (size_t)vsnprintf(&wb->buffer[wb->len], len, fmt, args);
	va_end(args);

	if (wrote >= len) {
		// there is bug in vsnprintf() and it returns
		// a number higher to len, but it does not
		// overflow the buffer.
		// our buffer overflow detector will log it
		// if it does.
		buffer_overflow_check(wb);

		debug(D_WEB_BUFFER, "web_buffer_sprintf(): increasing web_buffer at position %ld, size = %ld\n", wb->len, wb->size);
		buffer_need_bytes(wb, len + WEB_DATA_LENGTH_INCREASE_STEP);

		va_start(args, fmt);
		buffer_vsprintf(wb, fmt, args);
		va_end(args);
	}
	else
		wb->len += wrote;

	// the buffer is \0 terminated by vsnprintf
}

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



