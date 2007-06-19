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

/* $Id: dbaccess.h,v 1.5 2007/05/28 15:07:17 derek Exp $ */

typedef struct _db_handle_t { 
	char *user;
	char *pass;
	char *config;
	char **server;
	int server_count;
	int server_offset;
	MYSQL *db;
} db_handle_t;

db_handle_t * db_handle_init(const char *_user, const char *_pass, 
			const char *_server, const char *_config,void *userarg);
void db_handle_destroy(db_handle_t *);
json_value * dbexecute(db_handle_t *dbh, json_value *injson, apr_pool_t *mpool);

#endif //_DBACCESS_H_

