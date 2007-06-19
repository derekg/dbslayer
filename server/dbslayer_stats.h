#ifndef _DBSLAYER_STATS_H_
#include <apr_thread_proc.h>
#include <apr_queue.h>
#include <apr_poll.h>
#include <apr_atomic.h>
#include <apr_time.h>
#include <apr_file_info.h>
#include <apr_strings.h>

/* $Id: dbslayer_stats.h,v 1.3 2007/05/09 20:55:01 derek Exp $ */

typedef struct _dbslayer_stats_t {
  int *hits;
  time_t *slices;
	time_t start_time;
	int total_requests;
	int offset;
	apr_thread_mutex_t *mutex; 
	apr_pool_t *mpool;
	int nslice;
	int tslice;
} dbslayer_stats_t;

dbslayer_stats_t * dbslayer_stat_init(apr_pool_t *mpool,int nsclice, int tslice);
dbslayer_stats_t * dbslayer_stat_init(apr_pool_t *mpool,int nsclice, int tslice) ;
void dbslayer_stats_get(dbslayer_stats_t *istats, dbslayer_stats_t *out);
void dbslayer_stats_tick(dbslayer_stats_t *stats);
void dbslayer_stat_update(dbslayer_stats_t *stats);
void dbslayer_stats_timer_thread(apr_pool_t *mpool,dbslayer_stats_t *stats);
char * dbslayer_stats_tojson(dbslayer_stats_t *istats,apr_pool_t *mpool);
#endif //_DBSLAYER_STATS_H_
