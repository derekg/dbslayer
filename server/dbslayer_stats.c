#include "simplejson.h" 
#include "dbslayer_stats.h"

/* $Id: dbslayer_stats.c,v 1.4 2007/05/09 20:55:01 derek Exp $ */

dbslayer_stats_t * dbslayer_stat_init(apr_pool_t *mpool,int nslice, int tslice) { 
	apr_status_t  status;
	apr_pool_t *lpool;
	dbslayer_stats_t *stats;
	status = apr_pool_create(&lpool,mpool);
	stats = apr_palloc(lpool,sizeof(dbslayer_stats_t));	
	stats->mpool = lpool;
	status = apr_thread_mutex_create(&stats->mutex,APR_THREAD_MUTEX_DEFAULT,stats->mpool);
	stats->offset = stats->total_requests = 0;
	stats->nslice = nslice  + 1;
	stats->tslice = tslice;
	stats->hits = apr_pcalloc(stats->mpool,sizeof(int) * stats->nslice);
	stats->slices = apr_pcalloc(stats->mpool,sizeof(time_t) * stats->nslice);
	stats->start_time = stats->slices[stats->offset] = apr_time_now() / (1000*1000);
	return stats;
}

void dbslayer_stat_update(dbslayer_stats_t *stats) { 
	apr_thread_mutex_lock(stats->mutex);
	(stats->hits[stats->offset])++;	
	(stats->total_requests)++;
	apr_thread_mutex_unlock(stats->mutex);
}

void dbslayer_stats_get(dbslayer_stats_t *istats, dbslayer_stats_t *ostats) { 
	apr_thread_mutex_lock(istats->mutex);
	memcpy(ostats,istats,sizeof(dbslayer_stats_t));
	apr_thread_mutex_unlock(istats->mutex);
}
void dbslayer_stats_tick(dbslayer_stats_t *stats) { 
	apr_thread_mutex_lock(stats->mutex);
	stats->offset++;
	if(stats->offset == stats->nslice ) { stats->offset = 0; }
	stats->hits[stats->offset] = 0;
	stats->slices[stats->offset] = apr_time_now() / (1000*1000);
	apr_thread_mutex_unlock(stats->mutex);
}

void *dbslayer_stats_timer_thread_run(apr_thread_t *mthread, void *x) { 
	dbslayer_stats_t *stats = (dbslayer_stats_t*) x;
	while(1) {
		apr_sleep(1000*1000 * stats->tslice);
		dbslayer_stats_tick(stats);
	}
	return NULL;
}

void dbslayer_stats_timer_thread(apr_pool_t *mpool,dbslayer_stats_t *stats) { 
	apr_threadattr_t *thread_attr;
	apr_thread_t *thread;
	apr_threadattr_create(&thread_attr,mpool);
	apr_threadattr_detach_set(thread_attr,1); // detach
	apr_threadattr_stacksize_set(thread_attr,4096*10);
	apr_thread_create(&thread,thread_attr,dbslayer_stats_timer_thread_run,stats,mpool);
}

char * dbslayer_stats_tojson(dbslayer_stats_t *istats,apr_pool_t *mpool) { 
	dbslayer_stats_t stats;
	dbslayer_stats_get(istats,&stats);

	json_value *container = json_create_object(mpool);
	json_add_object(container,"total_requests",json_create_long(mpool,stats.total_requests));
	json_value *hits = json_create_array(mpool,stats.nslice);
	json_value *slices = json_create_array(mpool,stats.nslice);
	int i;
	for( i  = stats.offset+1; i < stats.nslice; i++) { 
		if (stats.slices[i] != 0) { 
			json_append_array(hits,json_create_long(mpool,stats.hits[i]));
			json_append_array(slices,json_create_long(mpool,stats.slices[i]));
		}
	}
	for( i = 0; i < stats.offset; i++) { 
		if (stats.slices[i] != 0) { 
			json_append_array(hits,json_create_long(mpool,stats.hits[i]));
			json_append_array(slices,json_create_long(mpool,stats.slices[i]));
		}
	}
	json_add_object(container,"hits",hits);
	json_add_object(container,"slices",slices);
	json_add_object(container,"start_time",json_create_long(mpool,stats.start_time));
	json_add_object(container,"current_time",json_create_long(mpool,apr_time_now() / (1000*1000)));
	return json_serialize(mpool,container);
}
