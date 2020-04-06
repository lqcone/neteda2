#include<stdarg.h>
#include<time.h>

#ifndef NETDATA_WEB_BUFFER_H
#define NETDATA_WEB_BUFFER_H 1

typedef struct web_buffer {
	size_t size;
	size_t len;
	char* buffer;
} BUFFER;


#define buffer_flush(wb) wb->buffer[wb->len=0]='\0';

extern const char* buffer_tostring(BUFFER *wb);

extern BUFFER* buffer_create(long size);




#endif