#ifndef _SLAYER_HTTP_SERVER_H_
#define _SLAYER_HTTP_SERVER_H_

#include <apr_atomic.h>
#include <apr_file_info.h>
#include <apr_general.h>
#include <apr_network_io.h>
#include <apr_poll.h>
#include <apr_queue.h>
#include <apr_strings.h>
#include <apr_thread_proc.h>
#include <apr_thread_rwlock.h>
#include <apr_time.h>
#include <apr_uri.h>

#include "slayer_server_logging.h"
#include "slayer_server_stats.h"
#include "slayer_http_parse.h"

#define SLAYER_MT_TEXT_PLAIN "text/plain; charset=utf-8"

typedef struct _slayer_http_service_map_t _slayer_http_service_map_t; 

// SERVER 
typedef struct _slayer_http_server_t { 
	apr_pool_t *mpool;
	slayer_server_log_manager_t *lmanager;
	slayer_server_log_manager_t *elmanager;
	slayer_server_stats_t  *stats;
	apr_queue_t *in_queue;
	apr_queue_t *out_queue;
	volatile apr_uint32_t  shutdown;
	int argc;
	const char **argv;
	char *startup_args;
	char *hostname;
	char *config;
	char *debug ;
	char *basedir;
	char *logfile;
	char *elogfile;
	int thread_count;
	int port;
	int socket_timeout;
	int nslice;
	int tslice;
	_slayer_http_service_map_t **service_map;
	int service_map_size;
	int uri_size;
	const char *server_name;	
} slayer_http_server_t;

typedef struct _slayer_http_request_t {
	apr_pool_t *mpool;
	apr_time_t begin_request;
	slayer_http_request_parse_t *parse; // this needs to cleaned up
	apr_uri_t uri;
	char *message;
	char *message_marker;
	char *message_end;
	int payload_size;
	int response_code;
	int read_done;
	int done;
} slayer_http_request_t;

typedef struct _slayer_http_connection_t { 
	apr_pool_t *mpool;
	apr_socket_t *conn;
	apr_pollfd_t pollfd;
	slayer_http_request_t *request;
} slayer_http_connection_t;

typedef struct _slayer_http_service_t {
  char * help_string;
	void * (*service_global_init_func)(apr_pool_t *, int, char **);
	void   (*service_global_destroy_func)(void *);
	void * (*service_thread_init_func)(apr_pool_t *, void * /*global_config_chunk*/);
	void   (*service_thread_destroy_func)(void *);
	void * (*service_handler_func)(slayer_http_server_t *server, void *global_config, 
																	slayer_http_connection_t *client, void *local_config); 
} slayer_http_service_t;

typedef struct _slayer_http_service_map_t { 
	char **urls;
	slayer_http_service_t *service;
	void *global_config;
} slayer_http_service_map_t;

int slayer_server_run(int service_map_size, slayer_http_service_map_t **service_map, int argc, char **argv, int uri_size,const char *server_name);
int slayer_http_handle_response(slayer_http_server_t *server, slayer_http_connection_t *connection, const char *mime_type, const char *message, int message_size);
#endif //_SLAYER_HTTP_SERVER_H_
