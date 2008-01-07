#ifndef _DBSLAYER_H_
#define _DBSLAYER_H_

#include <apr_general.h>
#include <apr_network_io.h>
#include <apr_strings.h>
#include <apr_uri.h>

#define SLAYER_QUERY_PATH "/db"
#define SLAYER_ALT_QUERY_PATH "/db/"
#define SLAYER_SHUTDOWN_PATH "/shutdown"
#define SLAYER_STATS_PATH "/stats"
#define SLAYER_SCHEMA_PATH "/schema"
#define SLAYER_MAPPING_PATH "/map"
#define SLAYER_STATS_LOG_PATH "/stats/log"
#define SLAYER_STATS_ERROR_PATH "/stats/error"
#define SLAYER_STATS_ARGS_PATH "/stats/args"
#define SLAYER_LUA_INPUT_VAR   "json"
#define SLAYER_LUA_SCHEMA_VAR  "schema_json"
#define SLAYER_LUA_SERVER_VAR  "server"
/* $Id: dbslayer.h,v 1.2 2007/05/09 20:55:00 derek Exp $ */

typedef struct _manager_t { 
	char *user;
	char *pass;
	char *server;
	char *config;
} manager_t;


#endif
