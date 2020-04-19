#include<pthread.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>

#include"common.h"
#include"rrd.h"
#include"log.h"

int rrd_delete_unupdated_dimensions = 0;

int rrd_update_every = UPDATE_EVERY;
int rrd_default_history_entries = RRD_DEFAULT_HISTORY_ENTRIES;

RRDSET* rrdset_root = NULL;
pthread_rwlock_t rrdset_root_rwlock = PTHREAD_RWLOCK_INITIALIZER;

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
	tmp.id[RRD_ID_LENGTH_MAX]  = '\0';
	tmp.hash = (hash) ? hash : simple_hash(tmp.id);

	avl_search(&(rrdset_root_index), (avl*)&tmp, rrdset_iterator, (avl**)&result);
	return result;
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
}
