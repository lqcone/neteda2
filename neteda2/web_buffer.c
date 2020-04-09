#include<stdio.h>
#include<stdlib.h>

#include"web_buffer.h"
#include"log.h"


#define BUFFER_OVERFLOW_EOF "EOF"

static inline void buffer_overflow_init(BUFFER* b)
{
	b->buffer[b->size] = '\0';
	strcpy(&b->buffer[b->size + 1], BUFFER_OVERFLOW_EOF);
}

const char* buffer_tostring(BUFFER* wb) {
	
	wb->buffer[wb->len] = '\0';
	return(wb->buffer);
}

void buffer_vsprintf(BUFFER* wb, const char* fmt, va_list args)
{
	if (!fmt || !*fmt) return;

	buffer_need_bytes(wb, 1);

	size_t len = wb->size - wb->len;

	wb->len += vsnprintf(&wb->buffer[wb->len], len, fmt, args);

	//buffer_overflow_check(wb);

	// the buffer is \0 terminated by vsnprintf
}

void buffer_sprintf(BUFFER* wb, const char* fmt, ...)
{
	if (!fmt || !*fmt) return;

	buffer_need_bytes(wb, 1);

	size_t len = wb->size - wb->len, wrote;

	va_list args;
	va_start(args, fmt);
	wrote = (size_t)vsnprintf(&(wb->buffer[wb->len]), len, fmt, args);
	va_end(args);

	if (wrote >= len) {
		// there is bug in vsnprintf() and it returns
		// a number higher to len, but it does not
		// overflow the buffer.
		// our buffer overflow detector will log it
		// if it does.
		//buffer_overflow_check(wb);

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

void buffer_increase(BUFFER* b, size_t free_size_required)
{
	//buffer_overflow_check(b);

	size_t left = b->size - b->len;

	if (left >= free_size_required) return;

	size_t increase = free_size_required - left;
	if (increase < WEB_DATA_LENGTH_INCREASE_STEP) increase = WEB_DATA_LENGTH_INCREASE_STEP;

	debug(D_WEB_BUFFER, "Increasing data buffer from size %d to %d.", b->size, b->size + increase);

	b->buffer = realloc(b->buffer, b->size + increase + sizeof(BUFFER_OVERFLOW_EOF) + 2);
	if (!b->buffer) fatal("Failed to increase data buffer from size %d to %d.", b->size + sizeof(BUFFER_OVERFLOW_EOF) + 2, b->size + increase + sizeof(BUFFER_OVERFLOW_EOF) + 2);

	b->size += increase;

	buffer_overflow_init(b);
	//buffer_overflow_check(b);
}

