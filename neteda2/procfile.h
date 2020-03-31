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

// return the number of lines present
#define procfile_lines(ff) (ff->lines->len)

//return the number of words of the Nth line
#define procfile_linewords(ff,line) (((line)<procfile_lines(ff))?(ff)->lines->lines[line].words:0)

//rethren the Nth word of the file, or empty string
#define procfile_word(ff,word) (((word)<(ff)->words->len) ? (ff)->words->words[word]:"")

//return the first word of the Nth line,or empty string
#define procfile_line(ff,line) (((line)<procfile_lines(ff)) ? procfile_word(ff,ff->lines->lines[(line)].first):"")

//return the Nth word of the current line
#define procfile_lineword(ff,line,word) (((line)<procfile_lines(ff) && (word)<procfile_linewords(ff,(line))) ? procfile_word((ff),(ff)->lines->lines[line].first+word):"")
