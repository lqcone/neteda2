#include<pthread.h>

#include"avl.h"

#define UPDATE_EVERY 1
extern int rrd_update_every;

#define RRD_DEFAULT_HISTORY_ENTRIES 3600
extern int rrd_default_history_entries;

#define RRD_ID_LENGTH_MAX 1024


#define RRDSET_TYPE_STACKED 2


// ----------------------------------------------------------------------------
// memory mode

#define RRD_MEMORY_MODE_RAM_NAME "ram"
#define RRD_MEMORY_MODE_MAP_NAME "map"
#define RRD_MEMORY_MODE_SAVE_NAME "save"

#define RRD_MEMORY_MODE_RAM 0
#define RRD_MEMORY_MODE_MAP 1
#define RRD_MEMORY_MODE_SAVE 2


struct rrdset {

	char id[RRD_ID_LENGTH_MAX + 1];
	
	int enabled;

	uint32_t hash;

	pthread_rwlock_t rwlock;

	struct rrdset* next;

};
typedef struct rrdset RRDSET;

extern RRDSET* rrdset_root;
extern pthread_rwlock_t rrdset_root_rwlock;


extern RRDSET* rrdset_find_bytype(const char* type, const char* id);


extern RRDSET* rrdset_create(const char* type, const char* id, const char* name, const char* family, const char* context, const char* title, const char* units, long priority, int update_every, int chart_type);








//extern