#include<string.h>

#include"log.h"

char* url_decode(char* str) {
	char* pstr = str,
		* buf = malloc(strlen(str) + 1),
		* pbuf = buf;
	if (!buf)
		fatal("Cannot allocate memory.");
	while (*pstr) {
		if (*pstr = '%') {

		}
		else if (*pstr = '+') {

		}
		else
			*pbuf++ = *pstr;
		pstr++;
	}
	*pbuf = '\0';
	return buf;
}