#ifndef _DBACCESS_H_
#define _DBACCESS_H_

#include "simplejson.h"
#include "mysql.h"
#include "errmsg.h"

/** 
 * server ring -> random starting point but then just loop
 * when you loop around you know you are hosed
 *
 **/

/* $Id: dbaccess.h,v 1.6 2008/03/04 15:59:12 derek Exp $ */

typedef struct _db_handle_t { 
	char *user;
	char *pass;
	char *config;
	char **server;
	int server_count;
	int server_offset;
	MYSQL *db;
	json_skip_head_t *dblookup;
	apr_pool_t *mpool;
} db_handle_t;

db_handle_t * db_handle_init(const char *_user, const char *_pass, 
			const char *_server, const char *_config,void *userarg, int multidb);
void db_handle_destroy(db_handle_t *);
json_value * dbexecute(db_handle_t *dbh, json_value *injson, apr_pool_t *mpool);

#endif //_DBACCESS_H_

