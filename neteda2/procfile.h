#include<stdint.h>

typedef struct{
	uint32_t len;
	uint32_t size;
	char* words[];
} pfwords;

typedef struct {
	uint32_t words;
	uint32_t first;
} ffline;

typedef struct {
	uint32_t len;
	uint32_t size;
	ffline lines[];
} pflines;



typedef struct {
	char filename[FILENAME_MAX + 1];
	uint32_t flags;
	int fd;
	size_t len;
	size_t size;
	pflines* lines;
	pfwords* words;
	char separators[256];
	char data[];
} procfile;




extern procfile* procfile_open(const char* filename);
extern procfile* procfile_readall(procfile* ff);