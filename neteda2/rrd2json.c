

#include"web_buffer.h"
#include"rrd.h"

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