#ifndef _JSON_SKIP_H_
#define _JSON_SKIP_H_


#include <apr_file_io.h>
#include <apr_strings.h>
#include <apr_lib.h>

typedef int (*json_skip_cmp_t)(void *,void*) ;

typedef struct _json_skip_node_t { 
	unsigned char height;	
	void *key;
	void *data;
	struct _json_skip_node_t **next_list;
} json_skip_node_t;

typedef struct _json_skip_head_t { 
	unsigned char height;
	json_skip_node_t *node;
	json_skip_cmp_t node_cmp;
	unsigned int node_count;
	apr_pool_t *mpool;
} json_skip_head_t;

/** External **/
json_skip_head_t * json_skip_create(apr_pool_t *mpool, unsigned char height,json_skip_cmp_t node_cmp ); 
void  json_skip_put(json_skip_head_t *head, void *key, void *value); 
void* json_skip_get(json_skip_head_t *head, void *key);
void json_skip_replace(json_skip_head_t *head, void *key, void *value);
json_skip_node_t * json_skip_find(json_skip_head_t *head, void *key);
void json_skip_free(json_skip_head_t *head);

#endif //_JSON_SKIP_H_
