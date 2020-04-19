#include<sys/types.h>
#include<syscall.h>
#include<stdint.h>
#include<stdio.h>

#include"strsep.h"



char* global_host_prefix = "";


uint32_t simple_hash(const char* name) {
	unsigned char* s = (unsigned char*)name;
	uint32_t hval = 0x811c9dc5;

	// FNV-1a algorithm
	while (*s) {
		// multiply by the 32 bit FNV magic prime mod 2^32
		// gcc optimized
		hval += (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);

		// xor the bottom with the current octet
		hval ^= (uint32_t)*s++;
	}

	// fprintf(stderr, "HASH: %u = %s\n", hval, name);
	return hval;
}

char* mystrsep(char** ptr, char* s) {
	char* p = "";
	while (p && !p[0] && *ptr) p = strsep_lqc(ptr, s);
	return p;
}


char* trim(char* s)
{
	// skip leading spaces
	while (*s && isspace(*s)) s++;
	if (!*s || *s == '#') return NULL;

	// skip tailing spaces
	long c = (long)strlen(s) - 1;
	while (c >= 0 && isspace(s[c])) {
		s[c] = '\0';
		c--;
	}
	if (c < 0) return NULL;
	if (!*s) return NULL;
	return s;
}

pid_t gettid(void){
	return syscall(SYS_gettid);
}