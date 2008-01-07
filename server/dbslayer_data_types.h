#ifndef DBSLAYER_DATA_TYPES_H_
#define DBSLAYER_DATA_TYPES_H_
typedef struct _thread_shared_data_t {
	char *user;
	char *pass;
	char *config;
	char *input_filter_lua;
	char *output_filter_lua;
	char *mapper_lua;
#ifdef HAVE_APR_MEMCACHE_H
    apr_memcache_t *memcache;
    apr_memcache_server_t *memcache_server;
    apr_memcache_stats_t *memcache_stats;
    int memcache_force;
#endif
	char *serialized_mapping;
	char *serialized_schema;
	char *server;
	char *basedir;
	dbslayer_log_manager_t *lmanager;
	dbslayer_log_manager_t *elmanager;
	dbslayer_stats_t *stats;
	apr_queue_t *in_queue;
	apr_queue_t *out_queue;
	volatile apr_uint32_t shutdown;
	char *startup_args;

} thread_shared_data_t;

typedef struct _thread_uniq_data_t {
	unsigned int thread_number;
#ifdef HAVE_LUA5_1_LUA_H
	lua_State * lua_state;
#endif
#ifdef HAVE_LUA_H
	lua_State * lua_state;
#endif	
} thread_uniq_data_t;

typedef struct _thread_wrapper_data_t {
	thread_shared_data_t *shared;
	thread_uniq_data_t *uniq;
} thread_wrapper_data_t;

typedef struct _queue_data_t {
	apr_pool_t *mpool;
	apr_socket_t *conn;
	apr_time_t begin_request;
} queue_data_t;

#endif /*DBSLAYER_DATA_TYPES_H_*/
