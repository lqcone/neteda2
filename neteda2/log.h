#include<stdio.h>
#include<stdarg.h>


#define DEBUG (0)


extern unsigned long long debug_flags;



#define debug(type, args...) do { if(unlikely(!silent && (debug_flags & type))) debug_int(__FILE__, __FUNCTION__, __LINE__, ##args); } while(0)
#define info(args...)    info_int(__FILE__, __FUNCTION__, __LINE__, ##args)
#define error(args...)  error_int("ERROR",__FILE__, __FUNCTION__, __LINE__, ##args)



extern void log_date(FILE* out);
extern void debug_int(const char* file, const char* function, const unsigned long line, const char* fmt, ...);
extern void info_int(const char* file, const char* function, const unsigned long line, const char* fmt, ...);
