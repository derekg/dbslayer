#ifndef _DBSLAYER_LOGGING_H_
#define _DBSLAYER_LOGGING_H_

#include <apr_thread_proc.h>
#include <apr_queue.h>
#include <apr_poll.h>
#include <apr_atomic.h>
#include <apr_time.h>
#include <apr_file_info.h>
#include <apr_strings.h>
#include "simplejson.h"

/* $Id: dbslayer_logging.h,v 1.4 2007/05/09 20:55:00 derek Exp $ */

typedef struct _query_entry_t { 
	char *json_view;
} query_entry_t;

typedef struct _dbslayer_log_manager_t { 
	apr_pool_t *mpool;
	apr_thread_mutex_t *file_mutex; 
	apr_file_t *fhandle;
	query_entry_t *entries;
	int offset;
	int nentries;
	apr_thread_mutex_t *list_mutex; 
} dbslayer_log_manager_t;

int dbslayer_log_open(dbslayer_log_manager_t **manager,const char *filename, int nentries, apr_pool_t *mpool);
int dbslayer_log_message(dbslayer_log_manager_t *manager, const char *message);
int dbslayer_log_close(dbslayer_log_manager_t *manager);

int dbslayer_log_request(dbslayer_log_manager_t *manager, apr_pool_t *mpool, apr_socket_t *conn, 
		const char *request_line, int response_code, int nbytes_sent, apr_int64_t time_toservices);

int dbslayer_log_err_message(dbslayer_log_manager_t *manager,apr_pool_t *mpool,apr_socket_t *conn, 
		const char *request_line, const char * message);
///
void dbslayer_log_add_entry(dbslayer_log_manager_t *manager, apr_pool_t *mpool,
			const char *client_ip,apr_int64_t rtime,
			const char *request_line,int response_code, 
			int nbytes_sent, apr_int64_t time_toservice );

void dbslayer_log_add_error(dbslayer_log_manager_t *manager, apr_pool_t *mpool,
			const char *client_ip,apr_int64_t rtime,
			const char *request_line, const char *error_msg );

char * dbslayer_log_get_entries(dbslayer_log_manager_t *manager, apr_pool_t *mpool);
#endif /*_DBSLAYER_LOGGING_H_*/
