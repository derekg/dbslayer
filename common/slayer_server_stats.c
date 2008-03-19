#include "simplejson.h"
#include "slayer_server_stats.h"

/* $Id: slayer_server_stats.c,v 1.1 2008/02/29 00:18:52 derek Exp $ */

slayer_server_stats_t * slayer_server_stat_init(apr_pool_t *mpool,int nslice, int tslice) {
	apr_status_t  status;
	apr_pool_t *lpool;
	slayer_server_stats_t *stats;
	status = apr_pool_create(&lpool,mpool);
	stats = apr_palloc(lpool,sizeof(slayer_server_stats_t));
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

void slayer_server_stat_update(slayer_server_stats_t *stats) {
	apr_thread_mutex_lock(stats->mutex);
	(stats->hits[stats->offset])++;
	(stats->total_requests)++;
	apr_thread_mutex_unlock(stats->mutex);
}

void slayer_server_stats_get(slayer_server_stats_t *istats, slayer_server_stats_t *ostats) {
	apr_thread_mutex_lock(istats->mutex);
	memcpy(ostats,istats,sizeof(slayer_server_stats_t));
	apr_thread_mutex_unlock(istats->mutex);
}
void slayer_server_stats_tick(slayer_server_stats_t *stats) {
	apr_thread_mutex_lock(stats->mutex);
	stats->offset++;
	if (stats->offset == stats->nslice ) {
		stats->offset = 0;
	}
	stats->hits[stats->offset] = 0;
	stats->slices[stats->offset] = apr_time_now() / (1000*1000);
	apr_thread_mutex_unlock(stats->mutex);
}

void *slayer_server_stats_timer_thread_run(apr_thread_t *mthread, void *x) {
	slayer_server_stats_t *stats = (slayer_server_stats_t*) x;
	while (1) {
		apr_sleep(1000*1000 * stats->tslice);
		slayer_server_stats_tick(stats);
	}
	return NULL;
}

void slayer_server_stats_timer_thread(apr_pool_t *mpool,slayer_server_stats_t *stats) {
	apr_threadattr_t *thread_attr;
	apr_thread_t *thread;
	apr_threadattr_create(&thread_attr,mpool);
	apr_threadattr_detach_set(thread_attr,1); // detach
	apr_threadattr_stacksize_set(thread_attr,4096*10);
	apr_thread_create(&thread,thread_attr,slayer_server_stats_timer_thread_run,stats,mpool);
}

char * slayer_server_stats_tojson(slayer_server_stats_t *istats,apr_pool_t *mpool) {
	slayer_server_stats_t stats;
	slayer_server_stats_get(istats,&stats);

	json_value *container = json_object_create(mpool);
	json_object_add(container,"total_requests",json_long_create(mpool,stats.total_requests));
	json_value *hits = json_array_create(mpool,stats.nslice);
	json_value *slices = json_array_create(mpool,stats.nslice);
	int i;
	for ( i  = stats.offset+1; i < stats.nslice; i++) {
		if (stats.slices[i] != 0) {
			json_array_append(hits,json_long_create(mpool,stats.hits[i]));
			json_array_append(slices,json_long_create(mpool,stats.slices[i]));
		}
	}
	for ( i = 0; i < stats.offset; i++) {
		if (stats.slices[i] != 0) {
			json_array_append(hits,json_long_create(mpool,stats.hits[i]));
			json_array_append(slices,json_long_create(mpool,stats.slices[i]));
		}
	}
	json_object_add(container,"hits",hits);
	json_object_add(container,"slices",slices);
	json_object_add(container,"start_time",json_long_create(mpool,stats.start_time));
	json_object_add(container,"current_time",json_long_create(mpool,apr_time_now() / (1000*1000)));
	return json_serialize(mpool,container);
}
