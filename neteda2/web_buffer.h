#include<stdarg.h>
#include<time.h>

typedef struct web_buffer {
	size_t size;
	size_t len;
	char* buffer;
} BUFFER ;




extern BUFFER* buffer_create(long size);