#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<pthread.h>
#include<sys/select.h>
#include<sys/time.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<errno.h>
#include<netinet/tcp.h>
#include<netinet/in.h>

#include"web_client.h"
#include"log.h"
#include"web_buffer.h"
#include"strsep.h"
#include"url.h"
#include"common.h"
#include"config.h"
#include"rrd2json.h"
#include "rrd.h"


#define INITAL_WEB_DATA_LENGTH 16384
#define WEB_REQUEST_LENGTH 16384

int web_client_timeout = DEFAULT_DISCONNECT_IDLE_WEB_CLIENTS_AFTER_SECOND;
int web_enable_gzip = 1;


struct web_client* web_clients = NULL;
unsigned long long web_clients_count = 0;


struct web_client* web_client_create(int listener) {
	struct web_client* w;
	info("web_client_create started.");
	w = calloc(1, sizeof(struct web_client));
	if (!w) {
		error("Cannot allocate new web_client memory .");
		return NULL;
	}

	w->id = ++web_clients_count;
	w->mode = WEB_CLIENT_MODE_NORMAL;

	{
		struct sockaddr* sadr;
		socklen_t addrlen;

		sadr = (struct sockaddr*) & w->clientaddr;
		addrlen = sizeof(w->clientaddr);

		w->ifd = accept(listener, sadr, &addrlen);
		if (w->ifd == -1) {
			error("Cannot accept  newo incoming connection.",w->id);
			free(w);
			return NULL;
		}
		w->ofd = w->ifd;        //输入输出通用同一套接字


		int flag = 1;
		if(setsockopt(w->ifd,SOL_SOCKET,SO_KEEPALIVE,(char *) &flag,sizeof(int))!=0)
			error("Cannot set SO_KEEPALIVE on socket.", w->id);



	}

	w->response.data = buffer_create(INITAL_WEB_DATA_LENGTH);
	if (!w->response.data) {
		close(w->ifd);
		free(w);
		return NULL;
	}
	w->response.header = buffer_create(HTTP_RESPONSE_HEADER_SIZE);
	if (!w->response.header) {
		close(w->ifd);
		free(w);
		return NULL;
	}

	w->response.header_output = buffer_create(HTTP_RESPONSE_HEADER_SIZE);
	if (!w->response.header_output) {
		close(w->ifd);
		free(w);
		return NULL;
	}


	w->wait_receive = 1;
	if (web_clients)web_clients->prev = w;
	w->next = web_clients;
	web_clients = w;
	return w;
}

void web_client_reset(struct web_client* w) {

	//long sent = (w->mode == WEB_CLIENT_MODE_FILECOPY) ? w->response.rlen : w->response.data->len;

	//long size = (w->mode == WEB_CLIENT_MODE_FILECOPY) ? w->response.rlen : w->response.data->len;

	if (w->mode == WEB_CLIENT_MODE_FILECOPY) {

		close(w->ifd);
		w->ifd = w->ofd;
	}
	w->last_url[0] = '\0';

	w->mode = WEB_CLIENT_MODE_NORMAL;

	buffer_reset(w->response.header_output);
	buffer_reset(w->response.header);
	buffer_reset(w->response.data);
	w->response.rlen = 0;
	w->response.sent = 0;
	w->response.code = 0;

	w->wait_receive = 1;
	w->wait_send = 0;
}


struct web_client* web_client_free(struct web_client* w) {
	struct web_client* n = w->next;

	if (w->prev) w->prev->next = w->next;
	if (w->next) w->next->prev = w->prev;
	if (w == web_clients)web_clients = w->next;
	
	if (w->response.header_output) buffer_free(w->response.header_output);
	if (w->response.header) buffer_free(w->response.header);
	if (w->response.data) buffer_free(w->response.data);
	close(w->ifd);
	if (w->ofd != w->ifd) close(w->ofd);
	free(w);

	return n;
}

int mysendfile(struct web_client* w, char* filename) {
	static char* web_dir = NULL;
	if (!web_dir) web_dir = "/home/nick/projects/neteda2_web";

	while (*filename == '/') filename++;

	char webfilename[FILENAME_MAX + 1];
	snprintf(webfilename, FILENAME_MAX, "%s/%s", web_dir, filename);
	struct stat stat;
	if (lstat(webfilename, &stat) != 0) {
		return 404;
	}

	if ((stat.st_mode & S_IFMT) == S_IFDIR) {
		snprintf(webfilename, FILENAME_MAX + 1, "%s/index.html", filename);
		return mysendfile(w, webfilename);
	}

	//open the file
	w->ifd = open(webfilename, O_NONBLOCK, O_RDONLY);
	if (w->ifd == -1) {
		w->ifd = w->ofd;
		if (errno == EBUSY || errno == EAGAIN) {
			return 307;
		}

		else {
			return 404;
		}
	}
	// pick a Content-Type for the file
	if (strstr(filename, ".html") != NULL)	w->response.data->contenttype = CT_TEXT_HTML;
	else if (strstr(filename, ".js") != NULL)	w->response.data->contenttype = CT_APPLICATION_X_JAVASCRIPT;
	else if (strstr(filename, ".css") != NULL)	w->response.data->contenttype = CT_TEXT_CSS;
	else if (strstr(filename, ".xml") != NULL)	w->response.data->contenttype = CT_TEXT_XML;
	else if (strstr(filename, ".xsl") != NULL)	w->response.data->contenttype = CT_TEXT_XSL;
	else if (strstr(filename, ".txt") != NULL)  w->response.data->contenttype = CT_TEXT_PLAIN;
	else if (strstr(filename, ".svg") != NULL)  w->response.data->contenttype = CT_IMAGE_SVG_XML;
	else if (strstr(filename, ".ttf") != NULL)  w->response.data->contenttype = CT_APPLICATION_X_FONT_TRUETYPE;
	else if (strstr(filename, ".otf") != NULL)  w->response.data->contenttype = CT_APPLICATION_X_FONT_OPENTYPE;
	else if (strstr(filename, ".woff2") != NULL)  w->response.data->contenttype = CT_APPLICATION_FONT_WOFF2;
	else if (strstr(filename, ".woff") != NULL)  w->response.data->contenttype = CT_APPLICATION_FONT_WOFF;
	else if (strstr(filename, ".eot") != NULL)  w->response.data->contenttype = CT_APPLICATION_VND_MS_FONTOBJ;
	else if (strstr(filename, ".png") != NULL)  w->response.data->contenttype = CT_IMAGE_PNG;
	else if (strstr(filename, ".jpg") != NULL)  w->response.data->contenttype = CT_IMAGE_JPG;
	else if (strstr(filename, ".jpeg") != NULL)  w->response.data->contenttype = CT_IMAGE_JPG;
	else if (strstr(filename, ".gif") != NULL)  w->response.data->contenttype = CT_IMAGE_GIF;
	else if (strstr(filename, ".bmp") != NULL)  w->response.data->contenttype = CT_IMAGE_BMP;
	else if (strstr(filename, ".ico") != NULL)  w->response.data->contenttype = CT_IMAGE_XICON;
	else if (strstr(filename, ".icns") != NULL)  w->response.data->contenttype = CT_IMAGE_ICNS;
	else w->response.data->contenttype = CT_APPLICATION_OCTET_STREAM;

	w->mode = WEB_CLIENT_MODE_FILECOPY;
	w->wait_receive = 1;
	w->wait_send = 0;
	buffer_flush(w->response.data);
	w->response.rlen = stat.st_size;


	return 200;
}


#ifdef NETDATA_WITH_ZLIB
void web_client_enable_deflate(struct web_client* w) {
	if (w->response.zinitialized == 1) {
		//error("%llu:Compression has already be initialized for this client.", w->id);
		return;
	}

	if (w->response.sent) {
		error("%llu: Cannot enable compression in the middle of a conversation.", w->id);
		return;
	}

	w->response.zstream.zalloc = Z_NULL;
	w->response.zstream.zfree = Z_NULL;
	w->response.zstream.opaque = Z_NULL;

	w->response.zstream.next_in = (Bytef*)w->response.data->buffer;
	w->response.zstream.avail_in = 0;
	w->response.zstream.total_in = 0;

	w->response.zstream.next_out = w->response.zbuffer;
	w->response.zstream.avail_out = 0;
	w->response.zstream.total_out = 0;

	w->response.zstream.zalloc = Z_NULL;
	w->response.zstream.zfree = Z_NULL;
	w->response.zstream.opaque = Z_NULL;

	//	if(deflateInit(&w->response.zstream, Z_DEFAULT_COMPRESSION) != Z_OK) {
	//		error("%llu: Failed to initialize zlib. Proceeding without compression.", w->id);
	//		return;
	//	}

		// Select GZIP compression: windowbits = 15 + 16 = 31
	if (deflateInit2(&w->response.zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
		error("%llu: Failed to initialize zlib. Proceeding without compression.", w->id);
		return;
	}

	w->response.zsent = 0;
	w->response.zoutput = 1;
	w->response.zinitialized = 1;

	//debug(D_DEFLATE, "%llu: Initialized compression.", w->id);
}
#endif

uint32_t web_client_api_request_v1_data_format(char* name) {

	if (!strcmp(name, DATASOURCE_FORMAT_DATATABLE_JSON))
		return DATASOURCE_DATATABLE_JSON;
	else if (!strcmp(name, DATASOURCE_FORMAT_DATATABLE_JSONP))
		return DATASOURCE_DATATABLE_JSONP;
	else if (!strcmp(name, DATASOURCE_FORMAT_JSON))
		return DATASOURCE_JSON;

}

int web_client_api_request_v1_charts(struct web_client* w, char* url) {

	buffer_flush(w->response.data);
	w->response.data->contenttype = CT_APPLICATION_JSON;
	rrd_stats_api_v1_charts(w->response.data);
	return 200;
}

int web_client_api_request_v1_data(struct web_client* w, char* url) {

	int ret = 400;

	buffer_flush(w->response.data);

	char* chart = NULL,*points_str = NULL;
	
	uint32_t format = DATASOURCE_JSON;

	while (url) {

		char* value = mystrsep(&url, "?&[]");
		if (!value || !*value) continue;
		char* name = mystrsep(&value, "=");
		if (!name || !*name) continue;
		if (!value || !*value) continue;

		if (!strcmp(name, "chart")) chart = value;
		else if (!strcmp(name, "format")) {
			format = web_client_api_request_v1_data_format(name);
		}
		else if (!strcmp(name, "points"))
			points_str = value;


	}

	if (!chart || !*chart) {
		buffer_sprintf(w->response.data, "No chart id is given at the request.");
		goto cleanup;
	}

	RRDSET* st = rrdset_find(chart);
	//if (!st) st = rrdset_find_byname(chart);
	if (!st) {
		buffer_sprintf(w->response.data, "Chart '%s' is not fount.", chart);
		ret = 404;
		goto cleanup;
	}

	int points = (points_str && *points_str) ? atoi(points_str) : 0;

	ret = rrd2format(st, w->response.data, format, points);

cleanup:
	return ret;
}

int web_client_api_request_v1(struct web_client* w, char* url) {

	//get the commond
	char* tok = mystrsep(&url, "/?&");
	if (tok && *tok) {
		if (strcmp(tok, "data") == 0) {
			//开始传输数据
			return web_client_api_request_v1_data(w, url);
		}
		else if (strcmp(tok, "charts") == 0) {
			return web_client_api_request_v1_charts(w, url);
		}
	}
	else {
		buffer_flush(w->response.data);
		buffer_sprintf(w->response.data, "API v1 command?");
		return 400;

	}

}

int web_client_api_request(struct web_client* w, char* url) {
	
	//get the api version
	char* tok = mystrsep(&url, "/?&");
	if (tok && *tok) {
		if (strcmp(tok, "v1") == 0) {
			return web_client_api_request_v1(w, url);
		}
		else {
			buffer_flush(w->response.data);
			buffer_sprintf(w->response.data, "Unsuported API version %s", tok);
			return 404;
		}
	}
	else {
		buffer_flush(w->response.data);
		buffer_sprintf(w->response.data, "Whitch API version?");
		return 400;
	}


}

void web_client_process(struct web_client *w){
	
	int code = 500;
	ssize_t bytes;
	int enable_gzip = 0;

	if (strstr(w->response.data->buffer, "\r\n\r\n")) {
		
		if (strcasestr(w->response.data->buffer, "Connection: keep-alive")) w->keepalive = 1;
		else w->keepalive = 0;
#ifdef NETDATA_WITH_ZLIB
		if (web_enable_gzip && strstr(w->response.data->buffer, "gzip"))
			enable_gzip = 1;
#endif
		
		int datasource_type = DATASOURCE_DATATABLE_JSONP;
		char* pointer_to_free = NULL;

		char* buf = (char*)buffer_tostring(w->response.data);
		char* url = NULL;
		
		char *tok = strsep_lqc(&buf, " \r\n");
		if (buf&&strcmp(tok, "GET") == 0) {
			tok = strsep_lqc(&buf, " \r\n");
			pointer_to_free=url = url_decode(tok);
		}

		w->last_url[0] = '\0';

		if (w->mode == WEB_CLIENT_MODE_OPTIONS) {

		}
		else if (url) {
#ifdef NETDATA_WITH_ZLIB
			if (enable_gzip)
				web_client_enable_deflate(w);
#endif
			


			strncpy(w->last_url, url, URL_MAX);
			w->last_url[URL_MAX] = '\0';
			tok = mystrsep(&url, "/?");
			if (tok && *tok) {
 				if (strcmp(tok, "api") == 0) {
					datasource_type = DATASOURCE_JSON;
					code = web_client_api_request(w, url);
				}

				else {
					char filename[FILENAME_MAX + 1];
					url = filename;
					strncpy(filename, w->last_url, FILENAME_MAX);
					filename[FILENAME_MAX] = '\0';
					tok = mystrsep(&url, "?");
					buffer_flush(w->response.data);
					code = mysendfile(w, (tok && *tok) ? tok : "/");
				}
			}
			else {
				char filename[FILENAME_MAX + 1];
				url = filename;
				strncpy(filename, w->last_url, FILENAME_MAX);
				filename[FILENAME_MAX] = '\0';
				tok = mystrsep(&url, "?");
				buffer_flush(w->response.data);
				code = mysendfile(w, (tok && *tok) ? tok : "/");
			}
		}

		//free url decode() buffer
		if (pointer_to_free) {
			free(pointer_to_free);
			pointer_to_free = NULL;
		}

	}

	w->response.data->date = time(NULL);
	w->response.code = code;

	char* content_type_string;
	switch (w->response.data->contenttype) {
	case CT_TEXT_HTML:
		content_type_string = "text/html; charset=utf-8";
		break;

	case CT_APPLICATION_XML:
		content_type_string = "application/xml; charset=utf-8";
		break;

	case CT_APPLICATION_JSON:
		content_type_string = "application/json; charset=utf-8";
		break;

	case CT_APPLICATION_X_JAVASCRIPT:
		content_type_string = "application/x-javascript; charset=utf-8";
		break;

	case CT_TEXT_CSS:
		content_type_string = "text/css; charset=utf-8";
		break;

	case CT_TEXT_XML:
		content_type_string = "text/xml; charset=utf-8";
		break;

	case CT_TEXT_XSL:
		content_type_string = "text/xsl; charset=utf-8";
		break;

	case CT_APPLICATION_OCTET_STREAM:
		content_type_string = "application/octet-stream";
		break;

	case CT_IMAGE_SVG_XML:
		content_type_string = "image/svg+xml";
		break;

	case CT_APPLICATION_X_FONT_TRUETYPE:
		content_type_string = "application/x-font-truetype";
		break;

	case CT_APPLICATION_X_FONT_OPENTYPE:
		content_type_string = "application/x-font-opentype";
		break;

	case CT_APPLICATION_FONT_WOFF:
		content_type_string = "application/font-woff";
		break;

	case CT_APPLICATION_FONT_WOFF2:
		content_type_string = "application/font-woff2";
		break;

	case CT_APPLICATION_VND_MS_FONTOBJ:
		content_type_string = "application/vnd.ms-fontobject";
		break;

	case CT_IMAGE_PNG:
		content_type_string = "image/png";
		break;

	case CT_IMAGE_JPG:
		content_type_string = "image/jpeg";
		break;

	case CT_IMAGE_GIF:
		content_type_string = "image/gif";
		break;

	case CT_IMAGE_XICON:
		content_type_string = "image/x-icon";
		break;

	case CT_IMAGE_BMP:
		content_type_string = "image/bmp";
		break;

	case CT_IMAGE_ICNS:
		content_type_string = "image/icns";
		break;

	default:
	case CT_TEXT_PLAIN:
		content_type_string = "text/plain; charset=utf-8";
		break;
	}


	char* code_msg;
	switch (code) {
	case 200:
		code_msg = "OK";
		break;

	case 307:
		code_msg = "Temporary Redirect";
		break;

	case 400:
		code_msg = "Bad Request";
		break;

	case 403:
		code_msg = "Forbidden";
		break;

	case 404:
		code_msg = "Not Found";
		break;

	default:
		code_msg = "Internal Server Error";
		break;
	}

	char date[100];
	struct tm tmbuf, * tm = gmtime_r(&w->response.data->date, &tmbuf);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", tm);

	buffer_sprintf(w->response.header_output,
		"HTTP/1.1 %d %s\r\n"
		"Connection: %s\r\n"
		"Server: NetData Embedded HTTP Server\r\n"
		"Content-Type: %s\r\n"
		"Access-Control-Allow-Origin: *\r\n"
		"Access-Control-Allow-Methods: GET, OPTIONS\r\n"
		"Access-Control-Allow-Headers: accept, x-requested-with\r\n"
		"Access-Control-Max-Age: 86400\r\n"
		"Date: %s\r\n"
		, code, code_msg
		, w->keepalive ? "keep-alive" : "close"
		, content_type_string
		, date
	);

	// if we know the content length, put it
	if (!w->response.zoutput && (w->response.data->len || w->response.rlen))
		buffer_sprintf(w->response.header_output,
			"Content-Length: %ld\r\n"
			, w->response.data->len ? w->response.data->len : w->response.rlen
		);
	else if (!w->response.zoutput)
		w->keepalive = 0;	// content-length is required for keep-alive

	if (w->response.zoutput) {
		buffer_strcat(w->response.header_output,
			"Content-Encoding: gzip\r\n"
			"Transfer-Encoding: chunked\r\n"
		);
	}
	buffer_strcat(w->response.header_output, "\r\n");     //报文头内容结束
	
	// disable TCP_NODELAY, to buffer the header
	int flag = 0;
	if (setsockopt(w->ofd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int)) != 0)
		error("%llu: failed to disable TCP_NODELAY on socket.", w->id);

	bytes = send(w->ofd, buffer_tostring(w->response.header_output), buffer_strlen(w->response.header_output), 0);

	// enable TCP_NODELAY, to send all data immediately at the next send()
	flag = 1;
	if (setsockopt(w->ofd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int)) != 0) 
		error("%llu: failed to enable TCP_NODELAY on socket.", w->id);

	//如果有数据到来，将数据发送出去
	if (w->response.data->len)
		w->wait_send = 1;
	else w->wait_send = 0;
}

long web_client_send_chunk_header(struct web_client* w, long len)
{
	debug(D_DEFLATE, "%llu: OPEN CHUNK of %d bytes (hex: %x).", w->id, len, len);
	char buf[1024];
	sprintf(buf, "%lX\r\n", len);
	ssize_t bytes = send(w->ofd, buf, strlen(buf), MSG_DONTWAIT);

	if (bytes > 0) debug(D_DEFLATE, "%llu: Sent chunk header %d bytes.", w->id, bytes);
	else if (bytes == 0) debug(D_DEFLATE, "%llu: Did not send chunk header to the client.", w->id);
	else debug(D_DEFLATE, "%llu: Failed to send chunk header to client.", w->id);

	return bytes;
}

long web_client_send_chunk_close(struct web_client* w)
{
	//debug(D_DEFLATE, "%llu: CLOSE CHUNK.", w->id);

	ssize_t bytes = send(w->ofd, "\r\n", 2, MSG_DONTWAIT);

	if (bytes > 0) debug(D_DEFLATE, "%llu: Sent chunk suffix %d bytes.", w->id, bytes);
	else if (bytes == 0) debug(D_DEFLATE, "%llu: Did not send chunk suffix to the client.", w->id);
	else debug(D_DEFLATE, "%llu: Failed to send chunk suffix to client.", w->id);

	return bytes;
}

long web_client_send_chunk_finalize(struct web_client* w)
{
	//debug(D_DEFLATE, "%llu: FINALIZE CHUNK.", w->id);

	ssize_t bytes = send(w->ofd, "\r\n0\r\n\r\n", 7, MSG_DONTWAIT);

	if (bytes > 0) debug(D_DEFLATE, "%llu: Sent chunk suffix %d bytes.", w->id, bytes);
	else if (bytes == 0) debug(D_DEFLATE, "%llu: Did not send chunk suffix to the client.", w->id);
	else debug(D_DEFLATE, "%llu: Failed to send chunk suffix to client.", w->id);

	return bytes;
}

#ifdef NETDATA_WITH_ZLIB
long web_client_send_deflate(struct web_client* w)
{
	long len = 0, t = 0;

	// when using compression,
	// w->response.sent is the amount of bytes passed through compression

	debug(D_DEFLATE, "%llu: web_client_send_deflate(): w->response.data->len = %d, w->response.sent = %d, w->response.zhave = %d, w->response.zsent = %d, w->response.zstream.avail_in = %d, w->response.zstream.avail_out = %d, w->response.zstream.total_in = %d, w->response.zstream.total_out = %d.", w->id, w->response.data->len, w->response.sent, w->response.zhave, w->response.zsent, w->response.zstream.avail_in, w->response.zstream.avail_out, w->response.zstream.total_in, w->response.zstream.total_out);

	if (w->response.data->len - w->response.sent == 0 && w->response.zstream.avail_in == 0 && w->response.zhave == w->response.zsent && w->response.zstream.avail_out != 0) {
		// there is nothing to send

		debug(D_WEB_CLIENT, "%llu: Out of output data.", w->id);

		// finalize the chunk
		if (w->response.sent != 0)
			t += web_client_send_chunk_finalize(w);

		// there can be two cases for this
		// A. we have done everything
		// B. we temporarily have nothing to send, waiting for the buffer to be filled by ifd

		if (w->mode == WEB_CLIENT_MODE_FILECOPY && w->wait_receive && w->ifd != w->ofd && w->response.rlen && w->response.rlen > w->response.data->len) {
			// we have to wait, more data will come
			debug(D_WEB_CLIENT, "%llu: Waiting for more data to become available.", w->id);
			w->wait_send = 0;
			return(0);
		}

		if (w->keepalive == 0) {
			debug(D_WEB_CLIENT, "%llu: Closing (keep-alive is not enabled). %ld bytes sent.", w->id, w->response.sent);
			errno = 0;
			return(-1);
		}

		// reset the client
		web_client_reset(w);
		debug(D_WEB_CLIENT, "%llu: Done sending all data on socket. Waiting for next request on the same socket.", w->id);
		return(0);
	}

	if (w->response.zhave == w->response.zsent) {
		// compress more input data

		// close the previous open chunk
		if (w->response.sent != 0) t += web_client_send_chunk_close(w);

		debug(D_DEFLATE, "%llu: Compressing %d new bytes starting from %d (and %d left behind).", w->id, (w->response.data->len - w->response.sent), w->response.sent, w->response.zstream.avail_in);

		// give the compressor all the data not passed through the compressor yet
		if (w->response.data->len > w->response.sent) {
#ifdef NETDATA_INTERNAL_CHECKS
			if ((long)w->response.sent - (long)w->response.zstream.avail_in < 0)
				error("internal error: avail_in is corrupted.");
#endif
			w->response.zstream.next_in = (Bytef*)&w->response.data->buffer[w->response.sent - w->response.zstream.avail_in];
			w->response.zstream.avail_in += (uInt)(w->response.data->len - w->response.sent);
		}

		// reset the compressor output buffer
		w->response.zstream.next_out = w->response.zbuffer;
		w->response.zstream.avail_out = ZLIB_CHUNK;

		// ask for FINISH if we have all the input
		int flush = Z_SYNC_FLUSH;
		if (w->mode == WEB_CLIENT_MODE_NORMAL
			|| (w->mode == WEB_CLIENT_MODE_FILECOPY && !w->wait_receive && w->response.data->len == w->response.rlen)) {
			flush = Z_FINISH;
			debug(D_DEFLATE, "%llu: Requesting Z_FINISH, if possible.", w->id);
		}
		else {
			debug(D_DEFLATE, "%llu: Requesting Z_SYNC_FLUSH.", w->id);
		}

		// compress
		if (deflate(&w->response.zstream, flush) == Z_STREAM_ERROR) {
			//error("%llu: Compression failed. Closing down client.", w->id);
			web_client_reset(w);
			return(-1);
		}

		w->response.zhave = ZLIB_CHUNK - w->response.zstream.avail_out;
		w->response.zsent = 0;

		// keep track of the bytes passed through the compressor
		w->response.sent = w->response.data->len;

		debug(D_DEFLATE, "%llu: Compression produced %d bytes.", w->id, w->response.zhave);

		// open a new chunk
		t += web_client_send_chunk_header(w, w->response.zhave);
	}

	debug(D_WEB_CLIENT, "%llu: Sending %d bytes of data (+%d of chunk header).", w->id, w->response.zhave - w->response.zsent, t);

	len = send(w->ofd, &w->response.zbuffer[w->response.zsent], (size_t)(w->response.zhave - w->response.zsent), MSG_DONTWAIT);
	if (len > 0) {
		w->response.zsent += len;
		if (t > 0) len += t;
		debug(D_WEB_CLIENT, "%llu: Sent %d bytes.", w->id, len);
	}
	else if (len == 0) debug(D_WEB_CLIENT, "%llu: Did not send any bytes to the client (zhave = %ld, zsent = %ld, need to send = %ld).", w->id, w->response.zhave, w->response.zsent, w->response.zhave - w->response.zsent);
	else debug(D_WEB_CLIENT, "%llu: Failed to send data to client. Reason: %s", w->id, strerror(errno));

	return(len);
}
#endif // NETDATA_WITH_ZLIB

long web_client_send(struct web_client* w) {
#ifdef NETDATA_WITH_ZLIB
	if (w->response.zoutput) return web_client_send_deflate(w);
#endif // NETDATA_WITH_ZLIB
	
	
	long bytes;

	if (w->response.data->len - w->response.sent == 0) {
		if (w->mode == WEB_CLIENT_MODE_FILECOPY && w->wait_receive && w->ifd != w->ofd && w->response.rlen && w->response.rlen > w->response.data->len) {
			// we have to wait, more data will come
			w->wait_send = 0;
			return(0);
		}
		web_client_reset(w);
		return 0;
	}
	//info("thread %d: sending data of ^s ", gettid(), w->last_url);
	bytes = send(w->ofd, &w->response.data->buffer[w->response.sent], w->response.data->len - w->response.sent, MSG_DONTWAIT);
	if (bytes > 0) {

		w->response.sent += bytes;
		//info("thread %d: sent data %d bytes", gettid(),bytes);

	}

	return bytes;
}

long web_client_receive(struct web_client* w) {
	// do we have any space for more data?
	buffer_need_bytes(w->response.data, WEB_REQUEST_LENGTH);

	long left = w->response.data->size - w->response.data->len;
	long bytes;
	if (w->mode == WEB_CLIENT_MODE_FILECOPY)
		bytes = read(w->ifd, &w->response.data->buffer[w->response.data->len], (ssize_t)(left - 1));
	else
		bytes = recv(w->ifd, &w->response.data->buffer[w->response.data->len], (size_t)(left - 1), MSG_DONTWAIT);
	if (bytes > 0) {
		w->response.data->len += bytes;
		w->response.data->buffer[w->response.data->len] = '\0';
		
		if (w->mode == WEB_CLIENT_MODE_FILECOPY) {
			w->wait_send = 1;
			if (w->response.rlen && w->response.data->len >= w->response.rlen)
				w->wait_receive = 0;

		}
	}
	else if (bytes == 0) {

		if (w->mode = WEB_CLIENT_MODE_FILECOPY) {
			
			w->wait_receive = 0;
		}
	}

		return bytes;
}



void* web_client_main(void* ptr) {

	struct timeval tv;
	struct web_client* w = ptr;
	int retval;

	fd_set ifds, ofds, efds;
	int fdmax = 0;
	 
	//info("web_client_main started");
	
	for (;;) {
		FD_ZERO(&ifds);
		FD_ZERO(&ofds);
		FD_ZERO(&efds);

		FD_SET(w->ifd, &efds);

		if (w->ifd != w->ofd)
			FD_SET(w->ofd, &efds);

		if (w->wait_receive) {
			FD_SET(w->ifd, &ifds);
			if (w->ifd > fdmax) fdmax = w->ifd;
		}

		if (w->wait_send) {
			FD_SET(w->ofd, &ofds);
			if (w->ofd > fdmax) fdmax = w->ofd;
		}

		tv.tv_sec = web_client_timeout;
		tv.tv_usec = 0;


		retval = select(fdmax + 1, &ifds, &ofds, &efds,&tv);


		if (retval == -1) { continue; }
		
		else if (!retval) { break; }

		if (FD_ISSET(w->ifd, &efds)) {

			break;
		}
		if (w->wait_send && FD_ISSET(w->ofd, &ofds)) {
			long bytes;
			if (bytes = web_client_send(w) < 0) {

				break;
			}
		}
		if (w->wait_receive && FD_ISSET(w->ifd, &ifds)) {
			long bytes;
			if (bytes = web_client_receive(w) < 0) {

				break;
			}
			if (w->mode == WEB_CLIENT_MODE_NORMAL) {
				web_client_process(w);
			}
		}
	}
	
	web_client_reset(w);
	w->obsolete = 1;

	info("web_client_main end,and wid is %d",w->id);


	return NULL;

}
