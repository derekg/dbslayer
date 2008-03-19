#ifndef _SLAYER_HTTP_PARSE_H_
#define _SLAYER_HTTP_PARSE_H_

#include <apr_general.h>

#define INVALID_HTTP_REQUEST -1

enum PARSE_REQUEST_STATE {
	PARSE_REQUEST_START = 0,
	PARSE_WS_BEFORE_URI,
	PARSE_REQUEST_METHOD,
	PARSE_REQUEST_URI,
	PARSE_WS_BEFORE_PROTOCOL,
	PARSE_REQUEST_PROTOCOL_HT,
	PARSE_REQUEST_PROTOCOL_HTT,
	PARSE_REQUEST_PROTOCOL_HTTP,
	PARSE_REQUEST_PROTOCOL_HTTP_SLASH,
	PARSE_REQUEST_PROTOCOL_HTTP_MAJOR,
	PARSE_REQUEST_PROTOCOL_HTTP_MAJOR_DOT,
	PARSE_REQUEST_PROTOCOL_HTTP_MAJOR_DOT_MINOR,
	PARSE_REQUEST_DONE_CR,
	PARSE_REQUEST_DONE_LF,
	PARSE_REQUEST_DONE
};
enum HTTP_METHOD { HTTP_METHOD_GET = 0, HTTP_METHOD_POST };
enum HTTP_PROTOCOL_VERSION { HTTP_10 = 0, HTTP_11 };

enum PARSE_HEADER_STATE {
	PARSE_HEADER_START =0,
	PARSE_HEADER_NAME,
	PARSE_HEADER_START_LF,
	PARSE_HEADER_SPACE_BEFORE_VALUE,
	PARSE_HEADER_VALUE,
	PARSE_HEADER_SPACE_AFTER_VALUE,
	PARSE_HEADER_DONE_LF,
	PARSE_HEADER_DONE,
};

typedef struct _slayer_http_request_parse_t {
	enum PARSE_REQUEST_STATE request_state;
	char method_string[6];
	int method_size;
	int uri_data_size;
	char *uri_data;
	char *uri_start;
	char *uri_end;
	enum HTTP_METHOD method;
	enum HTTP_PROTOCOL_VERSION version;
	enum PARSE_HEADER_STATE header_state;
	char *buffer;
	apr_size_t buffer_size;
	char *buffer_marker;
} slayer_http_request_parse_t;

int slayer_http_request_parse_init(apr_pool_t *mpool, slayer_http_request_parse_t **parse, int uri_size);
int slayer_http_request_parse_destroy(slayer_http_request_parse_t *parse);
int slayer_http_request_line_parse(slayer_http_request_parse_t *parse);
int slayer_http_request_header_parse(slayer_http_request_parse_t *parse);
#endif /*_SLAYER_HTTP_PARSE_H_*/
