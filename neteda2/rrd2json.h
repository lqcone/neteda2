#include"rrd.h"

// type of JSON generations
#define DATASOURCE_INVALID -1
#define DATASOURCE_JSON 0
#define DATASOURCE_DATATABLE_JSON 1
#define DATASOURCE_DATATABLE_JSONP 2
#define DATASOURCE_SSV 3
#define DATASOURCE_CSV 4
#define DATASOURCE_JSONP 5
#define DATASOURCE_TSV 6
#define DATASOURCE_HTML 7
#define DATASOURCE_JS_ARRAY 8
#define DATASOURCE_SSV_COMMA 9
#define DATASOURCE_CSV_JSON_ARRAY 10

#define DATASOURCE_FORMAT_JSON "json"
#define DATASOURCE_FORMAT_DATATABLE_JSON "datatable"
#define DATASOURCE_FORMAT_DATATABLE_JSONP "datasource"
#define DATASOURCE_FORMAT_JSONP "jsonp"
#define DATASOURCE_FORMAT_SSV "ssv"
#define DATASOURCE_FORMAT_CSV "csv"
#define DATASOURCE_FORMAT_TSV "tsv"
#define DATASOURCE_FORMAT_HTML "html"
#define DATASOURCE_FORMAT_JS_ARRAY "array"
#define DATASOURCE_FORMAT_SSV_COMMA "ssvcomma"
#define DATASOURCE_FORMAT_CSV_JSON_ARRAY "csvjsonarray"



extern void rrd_stats_api_v1_charts(BUFFER* wb);

extern int rrd2format(RRDSET* st, BUFFER* out, uint32_t format,long points);