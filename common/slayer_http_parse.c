#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "slayer_http_parse.h"

// NEED TO ADD IN VERSION THAT PLAYS NICE w/ apr mem pools
int slayer_http_request_parse_init(apr_pool_t *mpool, slayer_http_request_parse_t **parse, int uri_size) {
	*parse = apr_pcalloc(mpool,sizeof(slayer_http_request_parse_t));

	(*parse)->uri_data_size = uri_size;
	(*parse)->uri_end = (*parse)->uri_start = (*parse)->uri_data = apr_pcalloc(mpool, sizeof(char) *  (uri_size+1) );

	(*parse)->buffer = apr_palloc(mpool,sizeof(char) * uri_size);
	(*parse)->buffer_marker = NULL;
	(*parse)->buffer_size = uri_size;
	return 0;
}

int slayer_http_request_parse_destroy(slayer_http_request_parse_t *request) {
	return 0;
}

int slayer_http_request_line_parse(slayer_http_request_parse_t *parse) {
	enum PARSE_REQUEST_STATE state;
	char *p = NULL;

	state = parse->request_state;

	for (p = parse->buffer_marker; p < (parse->buffer + parse->buffer_size);p++) {
		switch (state) {
		case PARSE_REQUEST_START:
			if ( *p != 'G' && *p != 'P') {
				return INVALID_HTTP_REQUEST;
			}
			parse->method_string[parse->method_size] = *p;
			parse->method_size++;
			state = PARSE_REQUEST_METHOD;
			break;
		case PARSE_REQUEST_METHOD:
			if (parse->method_size > (sizeof(parse->method_string) -1) ) {
				return INVALID_HTTP_REQUEST;
			}
			if (*p == ' ') {
				switch (parse->method_size) {
				case 3:
					if (memcmp(parse->method_string,"GET",3)!=0) {
						return INVALID_HTTP_REQUEST;
					}
					parse->method = HTTP_METHOD_GET;
					state = PARSE_WS_BEFORE_URI;
					break;
				case 4:
					if (memcmp(parse->method_string,"POST",4)!=0) {
						return  INVALID_HTTP_REQUEST;
					}
					parse->method = HTTP_METHOD_POST;
					state = PARSE_WS_BEFORE_URI;
					break;
				default:
					return INVALID_HTTP_REQUEST;
				}
			} else {
				parse->method_string[parse->method_size] = *p;
				parse->method_size++;
			}
			break;
		case PARSE_WS_BEFORE_URI:
			switch ( *p) {
			case ' ':
				break;
			case '/':
				*(parse->uri_end) = *p;
				parse->uri_end++;
				state = PARSE_REQUEST_URI;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_URI:
			if ( (parse->uri_end - parse->uri_start) >= parse->uri_data_size) {
				return INVALID_HTTP_REQUEST;
			}
			switch (*p) {
			case ' ':
				state = PARSE_WS_BEFORE_PROTOCOL;
				break;
			case '\0':
				return INVALID_HTTP_REQUEST;
				break;
			default:
				*(parse->uri_end) = *p;
				parse->uri_end++;
				break;
			}
			break;
		case PARSE_WS_BEFORE_PROTOCOL:
			switch ( *p) {
			case ' ':
				break;
			case 'H':
				state = PARSE_REQUEST_PROTOCOL_HT;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_PROTOCOL_HT:
			switch ( *p) {
			case 'T':
				state = PARSE_REQUEST_PROTOCOL_HTT;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_PROTOCOL_HTT:
			switch ( *p) {
			case 'T':
				state = PARSE_REQUEST_PROTOCOL_HTTP;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_PROTOCOL_HTTP:
			switch ( *p) {
			case 'P':
				state = PARSE_REQUEST_PROTOCOL_HTTP_SLASH;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_PROTOCOL_HTTP_SLASH:
			switch ( *p) {
			case '/':
				state = PARSE_REQUEST_PROTOCOL_HTTP_MAJOR;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_PROTOCOL_HTTP_MAJOR:
			switch ( *p) {
			case '1':
				state = PARSE_REQUEST_PROTOCOL_HTTP_MAJOR_DOT;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_PROTOCOL_HTTP_MAJOR_DOT:
			switch ( *p) {
			case '.':
				state = PARSE_REQUEST_PROTOCOL_HTTP_MAJOR_DOT_MINOR;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_PROTOCOL_HTTP_MAJOR_DOT_MINOR:
			switch ( *p) {
			case '0':
				parse->version = HTTP_10;
				state = PARSE_REQUEST_DONE_CR;
				break;
			case '1':
				parse->version = HTTP_11;
				state = PARSE_REQUEST_DONE_CR;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_DONE_CR:
			switch ( *p) {
			case '\n':
				goto REQUEST_LINE_FINISHED;
				break;
			case '\r':
				state = PARSE_REQUEST_DONE_LF;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_DONE_LF:
			switch ( *p) {
			case '\n':
				goto REQUEST_LINE_FINISHED;
				break;
			default:
				return INVALID_HTTP_REQUEST;
			}
			break;
		case PARSE_REQUEST_DONE:
			return INVALID_HTTP_REQUEST;
		} // end of switch
	}//end of for loop

	parse->buffer_marker = p +1;
	parse->request_state = state;
	return 1;
REQUEST_LINE_FINISHED:
	parse->buffer_marker = p +1;
	parse->request_state = PARSE_REQUEST_DONE;
	return 0;
}

int check_header(char x) {
	if (tolower(x) >= 'a' && tolower(x) <= 'z') return 1;
	if (x == '_' || x == '-' || x == '.') return 1;
	if (isdigit(x)) return 1;
	return 0;
}

/** just validates it doesn't store it off for now **/
int slayer_http_request_header_parse( slayer_http_request_parse_t *parse) {
	enum PARSE_REQUEST_STATE state;
	char *p = NULL;
	state = parse->header_state;
	for (p = parse->buffer_marker; p < (parse->buffer + parse->buffer_size);p++) {
		switch (state) {
		case PARSE_HEADER_START:
			switch (*p) {
			case '\r':
				state = PARSE_HEADER_START_LF;
				break;
			case '\n':
				state = PARSE_HEADER_DONE;
				goto 	HEADER_SECTION_END;
				break;
			default:
				if (check_header(*p)) {
					//check valid range - mark begining
					state = PARSE_HEADER_NAME;
				} else {
					return -1;
				}
			}
			break;
		case PARSE_HEADER_START_LF:
			if (*p == '\n') {
				state = PARSE_HEADER_DONE;
				goto HEADER_SECTION_END;
			}
			return -1;
			break;
		case PARSE_HEADER_NAME:
			if ( check_header(*p) ) {
				break;
			} else {
				switch (*p) {
				case ':':
					state = PARSE_HEADER_SPACE_BEFORE_VALUE;
					break;
				case '\r':
					state = PARSE_HEADER_DONE_LF;
					break;
				case '\n':
					state = PARSE_HEADER_START;
					goto HEADER_END;
					break;
				default:
					//you got a bad header
					return -1;
				}
			}
			break;
		case PARSE_HEADER_SPACE_BEFORE_VALUE:
			switch (*p) {
			case ' ':
				break;
			case '\r':
				state = PARSE_HEADER_DONE_LF;
				break;
			case '\n':
				state = PARSE_HEADER_START;
				goto HEADER_END;
			}
			break;
		case PARSE_HEADER_VALUE:
			switch (*p) {
			case ' ':
				state = PARSE_HEADER_SPACE_AFTER_VALUE;
				break;
			case '\r':
				state = PARSE_HEADER_DONE_LF;
				break;
			case '\n':
				state = PARSE_HEADER_START;
				goto HEADER_END;
			default:
				break;
			}
			break;
		case PARSE_HEADER_SPACE_AFTER_VALUE:
			switch (*p) {
			case ' ':
				state = PARSE_HEADER_SPACE_AFTER_VALUE;
				break;
			case '\r':
				state = PARSE_HEADER_DONE_LF;
				break;
			case '\n':
				state = PARSE_HEADER_START;
				goto HEADER_END;
			default:
				state = PARSE_HEADER_VALUE;
				break;
			}
			break;
		case PARSE_HEADER_DONE_LF:
			if (*p == '\n') {
				state = PARSE_HEADER_START;
				goto HEADER_END;
			}
			return -1;
			break;
		case PARSE_HEADER_DONE:
			//printf("Should never see this\n");
			break;
		default:
			break;
		}//end of switch

	}//end of for loop
HEADER_END:
	parse->buffer_marker = p +1;
	parse->header_state = state;
	return 1;
HEADER_SECTION_END:
	parse->buffer_marker = p +1;
	parse->header_state = PARSE_HEADER_DONE;
	return 0;
}
