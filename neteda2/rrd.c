#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<errno.h>

#include"common.h"
#include"rrd.h"
#include"log.h"
#include"appconfig.h"

int rrd_delete_unupdated_dimensions = 0;

int rrd_update_every = UPDATE_EVERY;
int rrd_default_history_entries = RRD_DEFAULT_HISTORY_ENTRIES;

RRDSET* rrdset_root = NULL;
pthread_rwlock_t rrdset_root_rwlock = PTHREAD_RWLOCK_INITIALIZER;

int rrd_memory_mode = RRD_MEMORY_MODE_SAVE;



static int rrdset_iterator(avl* a) { if (a) {}; return 0; }

static int rrdset_compare(void* a, void* b) {
	if (((RRDSET*)a)->hash < ((RRDSET*)b)->hash) return -1;
	else if (((RRDSET*)a)->hash > ((RRDSET*)b)->hash) return 1;
	else return strcmp(((RRDSET*)a)->id, ((RRDSET*)b)->id);
}

avl_tree rrdset_root_index = {
		NULL,
		rrdset_compare,
#ifdef AVL_LOCK_WITH_MUTEX
		PTHREAD_MUTEX_INITIALIZER
#else
		PTHREAD_RWLOCK_INITIALIZER
#endif
};
static RRDSET* rrdset_index_find(const char* id, uint32_t hash) {
	RRDSET* result = NULL, tmp;
	strncpy(tmp.id, id, RRD_ID_LENGTH_MAX);
	tmp.id[RRD_ID_LENGTH_MAX] = '\0';
	tmp.hash = (hash) ? hash : simple_hash(tmp.id);

	avl_search(&(rrdset_root_index), (avl*)&tmp, rrdset_iterator, (avl**)&result);
	return result;
}

char* rrdset_strncpy_name(char* to, const char* from, int length)
{
	int i;
	for (i = 0; i < length && from[i]; i++) {
		if (from[i] == '.' || isalpha(from[i]) || isdigit(from[i])) to[i] = from[i];
		else to[i] = '_';
	}
	if (i < length) to[i] = '\0';
	to[length - 1] = '\0';

	return to;
}

char* rrdset_cache_dir(const char* id)
{
	char* ret = NULL;

	char* cache_dir = NULL;
		if (!cache_dir) {
			cache_dir = config_get("global", "cache directory", CACHE_DIR);
		}

	char b[FILENAME_MAX + 1];
	char n[FILENAME_MAX + 1];
	rrdset_strncpy_name(b, id, FILENAME_MAX);

	snprintf(n, FILENAME_MAX, "%s/%s", cache_dir, b);
	ret = config_get(id, "cache directory", n);

	if (rrd_memory_mode == RRD_MEMORY_MODE_MAP || rrd_memory_mode == RRD_MEMORY_MODE_SAVE) {
		int r = mkdir(ret, 0775);
		if (r != 0 && errno != EEXIST)
			error("Cannot create directory '%s'", ret);
	}

	return ret;
}


RRDSET* rrdset_find(const char* id) {

	RRDSET* st = rrdset_index_find(id, 0);
	return st;
}

RRDSET* rrdset_find_bytype(const char* type, const char *id) {
	char buf[RRD_ID_LENGTH_MAX + 1];
	strncpy(buf, type, RRD_ID_LENGTH_MAX - 1);
	buf[RRD_ID_LENGTH_MAX - 1] = '\0';
	strcat(buf, ".");
	int len = (int)strlen(buf);
	strncpy(&buf[len], id, (size_t)(RRD_ID_LENGTH_MAX - len));
	buf[RRD_ID_LENGTH_MAX] = '\0';

	return(rrdset_find(buf));
}



RRDSET* rrdset_create(const char* type, const char* id, const char* name, const char* family, const char* context, const char* title, const char* units, long priority, int update_every, int chart_type) {
	
	if (!type || !type[0]) {
		fatal("Cannot create rrd stats without a type.");
		return NULL;
	}

	if (!id || !id[0]) {
		fatal("Cannot create rrd stats without an id.");
		return NULL;
	}

	char fullid[RRD_ID_LENGTH_MAX + 1];
	char fullfilename[FILENAME_MAX + 1];
	RRDSET* st = NULL;

	snprintf(fullid, RRD_ID_LENGTH_MAX, "%s.%s", type, id);

	st = rrdset_find(fullid);
	if (st) {
		error("Cannot create rrd stats for '%s', it already exists.", fullid);
		return st;
	}

	char* cache_dir = rrdset_cache_dir(fullid);

}
