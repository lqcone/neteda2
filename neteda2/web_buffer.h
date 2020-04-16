#include<stdarg.h>
#include<time.h>
#include<stdint.h>

#ifndef NETDATA_WEB_BUFFER_H
#define NETDATA_WEB_BUFFER_H 1

#define WEB_DATA_LENGTH_INCREASE_STEP 16384


typedef struct web_buffer {
	size_t size;
	size_t len;
	char* buffer;
	uint8_t contenttype;
	time_t date;

} BUFFER;



// content-types
#define CT_APPLICATION_JSON				1
#define CT_TEXT_PLAIN					2
#define CT_TEXT_HTML					3
#define CT_APPLICATION_X_JAVASCRIPT		4
#define CT_TEXT_CSS						5
#define CT_TEXT_XML						6
#define CT_APPLICATION_XML				7
#define CT_TEXT_XSL						8
#define CT_APPLICATION_OCTET_STREAM		9
#define CT_APPLICATION_X_FONT_TRUETYPE	10
#define CT_APPLICATION_X_FONT_OPENTYPE	11
#define CT_APPLICATION_FONT_WOFF		12
#define CT_APPLICATION_FONT_WOFF2		13
#define CT_APPLICATION_VND_MS_FONTOBJ	14
#define CT_IMAGE_SVG_XML				15
#define CT_IMAGE_PNG					16
#define CT_IMAGE_JPG					17
#define CT_IMAGE_GIF					18
#define CT_IMAGE_XICON					19
#define CT_IMAGE_ICNS					20
#define CT_IMAGE_BMP					21

#define buffer_strlen(wb) ((wb)->len)
#define buffer_need_bytes(buffer, needed_free_size) do { if((buffer)->size - (buffer)->len < (size_t)(needed_free_size)) buffer_increase((buffer), (size_t)(needed_free_size)); } while(0)


#define buffer_flush(wb) wb->buffer[wb->len=0]='\0';

extern const char* buffer_tostring(BUFFER *wb);

extern void buffer_strcat(BUFFER* wb, const char* txt);

extern BUFFER* buffer_create(long size);
extern void buffer_free(BUFFER* B);

extern void buffer_sprintf(BUFFER* wb, const char* fmt, ...);





#endif