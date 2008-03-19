#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "json_skip.h"

const char * _json_skip_id = "$Id: json_skip.c,v 1.3 2008/03/07 15:15:52 derek Exp $";

static json_skip_node_t * json_skip_node_create(json_skip_head_t *head, void *key, void *data) {
	int r;
	json_skip_node_t *node = head->mpool == NULL ? malloc(sizeof(json_skip_node_t)) : apr_pcalloc(head->mpool,sizeof(json_skip_node_t));
	node->key = key;
	node->data = data;
	for(node->height = 1, r = rand(); node->height < head->height && r & 1 ; node->height++, r = r >> 1);
	node->next_list = head->mpool== NULL ? malloc(sizeof(json_skip_node_t*) * node->height) :apr_pcalloc(head->mpool,sizeof(json_skip_node_t*) * node->height);
	memset(node->next_list ,0,sizeof(json_skip_node_t*) * node->height);
	return node;
}

json_skip_head_t * json_skip_create(apr_pool_t *mpool,unsigned char height,json_skip_cmp_t node_cmp ){
	json_skip_head_t *head = mpool == NULL ? malloc(sizeof(json_skip_head_t)) : apr_pcalloc(mpool,sizeof(json_skip_head_t));
	head->height = height;
	head->node_count = 0;
	head->node_cmp = node_cmp;
	head->mpool = mpool;
	head->node = head->mpool == NULL ? malloc(sizeof(json_skip_node_t)) : apr_pcalloc(mpool,sizeof(json_skip_node_t));
	head->node->height = height;
	head->node->data = head->node->key = NULL;
	head->node->next_list  = head->mpool == NULL ? malloc(sizeof(json_skip_node_t*) * head->height): apr_palloc(head->mpool,sizeof(json_skip_node_t*) * head->height);
	memset(head->node->next_list ,0,sizeof(json_skip_node_t*) * head->height);
	return head;
} 

void json_skip_put(json_skip_head_t *head, void *key, void *value) {
	json_skip_node_t *new_node = json_skip_node_create(head,key,value);
	int height;
	json_skip_node_t *mark =  head->node;
	json_skip_node_t **fix = malloc(sizeof(json_skip_node_t*)* head->height);
	for(height = head->height -1 ; height >=0; height--) { 
		while( mark->next_list[height] !=NULL && head->node_cmp(new_node->key,mark->next_list[height]->key) > 0) { 
				mark = mark->next_list[height];
		}
		fix[height] = mark;
	}//end of for loop 
	for(height = new_node->height -1 ; height >=0; height--) { 
		new_node->next_list[height] = fix[height]->next_list[height];
		fix[height]->next_list[height] = new_node;
	}
	free(fix);
	head->node_count++;	
}

void json_skip_replace(json_skip_head_t *head, void *key, void *value) {
	json_skip_node_t *node = NULL;
	if( (node = json_skip_find(head,key)) !=NULL && head->node_cmp(key,node->key) == 0) {
			node->data = value;
	} else {
			json_skip_put(head,key,value);
	}
}

void * json_skip_get(json_skip_head_t *head, void *key) {
	int height;
	json_skip_node_t *mark =  head->node;
	for(height = head->height -1 ; height >=0; height--) { 
		while( mark->next_list[height] !=NULL && head->node_cmp(key,mark->next_list[height]->key) > 0) { 
				mark = mark->next_list[height];
		}
	}
	return (mark->next_list[0] !=NULL && head->node_cmp(key,mark->next_list[0]->key) == 0) ? 
			mark->next_list[0]->data : NULL;
}

json_skip_node_t * json_skip_find(json_skip_head_t *head, void *key) {
	int height;
	json_skip_node_t *mark =  head->node;
	for(height = head->height -1 ; height >=0; height--) { 
		while(mark->next_list[height] !=NULL && head->node_cmp(key,mark->next_list[height]->key) > 0 ) { 
				mark = mark->next_list[height];
		}
	}
	return mark->next_list[0];
}

void json_skip_free(json_skip_head_t *head) { 
	if(head->mpool == NULL) {
		json_skip_node_t *list = head->node->next_list[0];
		do { 
			json_skip_node_t *item = list;
			list = list->next_list[0];
			free(item->next_list);
			free(item);
		}while(list!=NULL);
		free(head->node->next_list);
		free(head->node);
		free(head);
	}
}
