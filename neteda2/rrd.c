#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<errno.h>

#include"common.h"
#include"rrd.h"
#include"log.h"
#include"appconfig.h"
#include"config.h"

#define RRD_DEFAULT_GAP_INTERPOLATIONS 1

int rrd_delete_unupdated_dimensions = 0;

int rrd_update_every = UPDATE_EVERY;
int rrd_default_history_entries = RRD_DEFAULT_HISTORY_ENTRIES;

RRDSET* rrdset_root = NULL;
pthread_rwlock_t rrdset_root_rwlock = PTHREAD_RWLOCK_INITIALIZER;

int rrd_memory_mode = RRD_MEMORY_MODE_SAVE;

#define rrdset_index_add(st) avl_insert(&rrdset_root_index, (avl *)(st))
#define rrdset_index_del(st) avl_remove(&rrdset_root_index, (avl *)(st))



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

static int rrddim_iterator(avl* a) { if (a) {}; return 0; }

static int rrddim_compare(void* a, void* b) {
	if (((RRDDIM*)a)->hash < ((RRDDIM*)b)->hash) return -1;
	else if (((RRDDIM*)a)->hash > ((RRDDIM*)b)->hash) return 1;
	else return strcmp(((RRDDIM*)a)->id, ((RRDDIM*)b)->id);
}

int rrdset_type_id(const char* name)
{
	if (unlikely(strcmp(name, RRDSET_TYPE_AREA_NAME) == 0)) return RRDSET_TYPE_AREA;
	else if (unlikely(strcmp(name, RRDSET_TYPE_STACKED_NAME) == 0)) return RRDSET_TYPE_STACKED;
	else if (unlikely(strcmp(name, RRDSET_TYPE_LINE_NAME) == 0)) return RRDSET_TYPE_LINE;
	return RRDSET_TYPE_LINE;
}

const char* rrdset_type_name(int chart_type)
{
	static char line[] = RRDSET_TYPE_LINE_NAME;
	static char area[] = RRDSET_TYPE_AREA_NAME;
	static char stacked[] = RRDSET_TYPE_STACKED_NAME;

	switch (chart_type) {
	case RRDSET_TYPE_LINE:
		return line;

	case RRDSET_TYPE_AREA:
		return area;

	case RRDSET_TYPE_STACKED:
		return stacked;
	}
	return line;
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

void rrdset_set_name(RRDSET* st, const char* name) {

	char b[CONFIG_MAX_VALUE + 1];
	char n[RRD_ID_LENGTH_MAX + 1];

	snprintf(n, RRD_ID_LENGTH_MAX, "%s.%s", st->type, name);
	rrdset_strncpy_name(b, n, CONFIG_MAX_VALUE);
	st->name = config_get(st->id, "name", b);
	st->hash_name = simple_hash(st->name);

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

RRDSET* rrdset_find_byname(const char* name) {
	RRDSET *st;

	return st;

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

	long entries = config_get_number(fullid, "history", rrd_default_history_entries);
	if (entries < 5) entries = config_set_number(fullid, "history", 5);
	if (entries > RRD_HISTORY_ENTRIES_MAX) entries = config_set_number(fullid, "history", RRD_HISTORY_ENTRIES_MAX);

	int enabled = config_get_boolean(fullid, "enabled", 1);
	if (!enabled) entries = 5;

	unsigned long size = sizeof(RRDSET);
	char* cache_dir = rrdset_cache_dir(fullid);

	if (!st) {
		st = calloc(1, size);
		if (!st) {
			fatal("Cannot allocate memory for RRD_STATS %s.%s", type, id);
			return NULL;
		}
	}

	st->memsize = size;
	st->entries = entries;
	st->update_every = update_every;

	strcpy(st->cache_filename, fullfilename);
	strcpy(st->magic, RRDSET_MAGIC);

	strcpy(st->id, fullid);
	st->hash = simple_hash(st->id);

	st->cache_dir = cache_dir;

	st->chart_type = rrdset_type_id(config_get(st->id, "chart type", rrdset_type_name(chart_type)));
	st->type = config_get(st->id, "type", type);
	st->family = config_get(st->id, "family", family ? family : st->type);
	st->context = config_get(st->id, "context", context ? context : st->id);
	st->units = config_get(st->id, "units", units ? units : "");

	st->priority = config_get_number(st->id, "priority", priority);
	st->enabled = enabled;

	st->isdetail = 0;
	st->debug = 0;

	st->last_collected_time.tv_sec = 0;
	st->last_collected_time.tv_usec = 0;
	st->counter_done = 0;

	st->gap_when_lost_iterations_above = (int)(
		config_get_number(st->id, "gap when lost iterations above", RRD_DEFAULT_GAP_INTERPOLATIONS) + 2);

	avl_init(&st->dimensions_index, rrddim_compare);


	pthread_rwlock_init(&st->rwlock, NULL);
	pthread_rwlock_wrlock(&rrdset_root_rwlock);

	if (name && *name) {}
	else rrdset_set_name(st, id);

	st->next = rrdset_root;
	rrdset_root = st;

	rrdset_index_add(st);

	pthread_rwlock_unlock(&rrdset_root_rwlock);

}
