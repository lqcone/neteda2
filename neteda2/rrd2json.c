#include<stdlib.h>
#include<stdint.h>

#include"web_buffer.h"
#include"rrd.h"
#include"rrd2json.h"

#define HOSTNAME_MAX 1024
char* hostname = "unknown";

void rrd_stats_api_v1_chart(RRDSET* st, BUFFER* wb) {
	pthread_rwlock_rdlock(&st->rwlock);

	buffer_sprintf(wb,
		"\t\t{\n"
		"\t\t\t\"id\": \"%s\",\n"
		"\t\t\t\"name\": \"%s\",\n"
		"\t\t\t\"type\": \"%s\",\n"
		"\t\t\t\"family\": \"%s\",\n"
		"\t\t\t\"context\": \"%s\",\n"
		"\t\t\t\"title\": \"%s\",\n"
		"\t\t\t\"priority\": %ld,\n"
		"\t\t\t\"enabled\": %s,\n"
		"\t\t\t\"units\": \"%s\",\n"
		"\t\t\t\"data_url\": \"/api/v1/data?chart=%s\",\n"
		"\t\t\t\"chart_type\": \"%s\",\n"
		"\t\t\t\"duration\": %ld,\n"
		"\t\t\t\"first_entry\": %lu,\n"
		"\t\t\t\"last_entry\": %lu,\n"
		"\t\t\t\"update_every\": %d,\n"
		"\t\t\t\"dimensions\": {\n"
		, st->id
		, st->name
		, st->type
		, st->family
		, st->context
		, st->title
		, st->priority
		, st->enabled ? "true" : "false"
		, st->units
		, st->name
		, rrdset_type_name(st->chart_type)
		, st->entries * st->update_every
		, rrdset_first_entry_t(st)
		, rrdset_last_entry_t(st)
		, st->update_every
	);

	buffer_sprintf(wb,
		"\n\t\t\t}\n"
		"\t\t}"
	);
	pthread_rwlock_unlock(&st->rwlock);
}

void rrd_stats_api_v1_charts(BUFFER* wb) {

	long c;
	RRDSET* st;

	buffer_sprintf(wb, "{\n"
		"\t\"hostname\": \"%s\""
		",\n\t\"update_every\": %d"
		",\n\t\"history\": %d"
		",\n\t\"charts\": {"
		, hostname
		, rrd_update_every
		, rrd_default_history_entries
	);
	pthread_rwlock_rdlock(&rrdset_root_rwlock);
	for (st = rrdset_root, c = 0; st; st = st->next) {
		if (st->enabled) {
			if (c) buffer_strcat(wb, ",");
			buffer_strcat(wb, "\n\t\t\"");
			buffer_strcat(wb, st->id);
			buffer_strcat(wb, "\": ");
			rrd_stats_api_v1_chart(st, wb);
			c++;
		}
	}
	pthread_rwlock_unlock(&rrdset_root_rwlock);
	buffer_strcat(wb, "\n\t}\n}\n");
}

typedef struct rrdresult {
	RRDSET* st;            //与结果关联的指标值

	int d;                // 维度的数量
	int n;                // 每个维度的数值数量

	calculated_number* v;   //指标值数组

	int has_st_lock;      //判断对应指标是否被读锁
} RRDR;

static void rrdr2json(RRDR* r, BUFFER* wb) {

	char kq[2] = "",					// key quote
		sq[2] = "",						// string quote
		pre_label[101] = "",			// before each label
		post_label[101] = "",			// after each label
		pre_date[101] = "",				// the beginning of line, to the date
		post_date[101] = "",			// closing the date
		pre_value[101] = "",			// before each value
		post_value[101] = "",			// after each value
		post_line[101] = "",			// at the end of each row
//		normal_annotation[201] = "",	// default row annotation
//		overflow_annotation[201] = "",	// overflow row annotation
		data_begin[101] = "",			// between labels and values
		finish[101] = "";				// at the end of everything

	int datatable = 0;
	if( datatable){}
	else {

		kq[0] = '"';
		sq[0] = '"';

		snprintf(pre_date, 100, "		[ ");
		snprintf(pre_label, 100, ", \"");
		snprintf(post_label, 100, "\"");
		snprintf(pre_value, 100, ", ");
		snprintf(post_line, 100, "]");
		snprintf(data_begin, 100, "],\n	%sdata%s:\n	[\n", kq, kq);
		snprintf(finish, 100, "\n	]\n}");

		buffer_sprintf(wb, "{\n	%slabels%s: [", kq, kq);
		buffer_sprintf(wb, "%stime%s", sq, sq);

	}

	long c, i;
	RRDDIM* rd;

	for (c = 0, i = 0, rd = r->st->dimensions; rd && c < r->d; c++, rd = rd->next) {
		buffer_strcat(wb, pre_label);
		buffer_strcat(wb, rd->name);
		buffer_strcat(wb, post_label);
		i++;
	}
	if (!i) {
		buffer_strcat(wb, pre_label);
		buffer_strcat(wb, "no data");
		buffer_strcat(wb, post_label);
	}

	// print the begin of row data
	buffer_strcat(wb, data_begin);

	// if all dimensions are hidden, print a null
	if (!i) {
		buffer_strcat(wb, finish);
		return;
	}




}

//创建RRDR，绑定RRDSET，指定每个维度值的数量n
static RRDR* rrdr_create(RRDSET* st,long n) {

	if (!st) {
		error("NULL value given!");
		return NULL;
	}

	RRDR* r = calloc(1, sizeof(RRDR));
	if (!r)
		goto cleanup;

	r->st = st; 

	RRDDIM* rd;
	for (rd = st->dimensions; rd; rd = rd->next)
		r->d++;

	r->n = n;

	r->v = malloc(n * r->d * sizeof(calculated_number));
	
	return r;
	
cleanup:
	error("Cannot alloate RRDR memmory for ... entries");
	return NULL;
}

RRDR* rrd2rrdr(RRDSET* st,long points) {

	RRDR* r = rrdr_create(st,points);
	if (!r) {
		return NULL;
	}


	return r;
}

int rrd2format(RRDSET* st, BUFFER* wb,uint32_t format) {

	RRDR* r = rrd2rrdr(st);
	if (!r) {
		buffer_strcat(wb, "Cannot generate output with these parameters on this chart.");
		return 500;
	}

	switch (format) {
	case DATASOURCE_JSON:
	default:
		rrdr2json(r, wb);
		break;
	}

}