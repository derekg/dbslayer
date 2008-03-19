#ifndef _SLAYER_LOGGING_H_
#define _SLAYER_LOGGING_H_

#include <apr_thread_proc.h>
#include <apr_queue.h>
#include <apr_poll.h>
#include <apr_atomic.h>
#include <apr_time.h>
#include <apr_file_info.h>
#include <apr_strings.h>
#include "simplejson.h"

/* $Id: slayer_server_logging.h,v 1.1 2008/02/29 00:18:53 derek Exp $ */

typedef struct _query_entry_t {
	char *json_view;
} query_entry_t;

typedef struct _slayer_server_log_manager_t {
	apr_pool_t *mpool;
	apr_thread_mutex_t *file_mutex;
	apr_file_t *fhandle;
	query_entry_t *entries;
	int offset;
	int nentries;
	apr_thread_mutex_t *list_mutex;
} slayer_server_log_manager_t;

int slayer_server_log_open(slayer_server_log_manager_t **manager,const char *filename, int nentries, apr_pool_t *mpool);
int slayer_server_log_message(slayer_server_log_manager_t *manager, const char *message);
int slayer_server_log_close(slayer_server_log_manager_t *manager);

int slayer_server_log_request(slayer_server_log_manager_t *manager, apr_pool_t *mpool, apr_socket_t *conn,
                               const char *request_line, int response_code, int nbytes_sent, apr_int64_t time_toservices);

int slayer_server_log_err_message(slayer_server_log_manager_t *manager,apr_pool_t *mpool,apr_socket_t *conn,
                                   const char *request_line, const char * message);
///
void slayer_server_log_add_entry(slayer_server_log_manager_t *manager, apr_pool_t *mpool,
                                  const char *client_ip,apr_int64_t rtime,
                                  const char *request_line,int response_code,
                                  int nbytes_sent, apr_int64_t time_toservice );

void slayer_server_log_add_error(slayer_server_log_manager_t *manager, apr_pool_t *mpool,
                                  const char *client_ip,apr_int64_t rtime,
                                  const char *request_line, const char *error_msg );

char * slayer_server_log_get_entries(slayer_server_log_manager_t *manager, apr_pool_t *mpool);
#endif /*_SLAYER_LOGGING_H_*/
