#ifndef _SLAYER_SERVER_STATS_H_
#define _SLAYER_SERVER_STATS_H_
#include <apr_thread_proc.h>
#include <apr_queue.h>
#include <apr_poll.h>
#include <apr_atomic.h>
#include <apr_time.h>
#include <apr_file_info.h>
#include <apr_strings.h>

/* $Id: slayer_server_stats.h,v 1.1 2008/02/29 00:18:53 derek Exp $ */

typedef struct _slayer_server_stats_t {
	int *hits;
	time_t *slices;
	time_t start_time;
	int total_requests;
	int offset;
	apr_thread_mutex_t *mutex;
	apr_pool_t *mpool;
	int nslice;
	int tslice;
} slayer_server_stats_t;

slayer_server_stats_t * slayer_server_stat_init(apr_pool_t *mpool,int nsclice, int tslice);
slayer_server_stats_t * slayer_server_stat_init(apr_pool_t *mpool,int nsclice, int tslice) ;
void slayer_server_stats_get(slayer_server_stats_t *istats, slayer_server_stats_t *out);
void slayer_server_stats_tick(slayer_server_stats_t *stats);
void slayer_server_stat_update(slayer_server_stats_t *stats);
void slayer_server_stats_timer_thread(apr_pool_t *mpool,slayer_server_stats_t *stats);
char * slayer_server_stats_tojson(slayer_server_stats_t *istats,apr_pool_t *mpool);
#endif //_SLAYER_SERVER_STATS_H_
