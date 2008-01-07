#ifndef _SIMPLEJSON_H_
#define _SIMPLEJSON_H_

/* $Id */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_tables.h>
#include <apr_hash.h>
#include <apr_strings.h>
#include <apr_time.h>


typedef struct { 
	const char *jstring;
	const char *offset;
	apr_pool_t *mpool;
} json_string; 

typedef enum { 
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_STRING,
	JSON_LONG,
	JSON_DOUBLE,
	JSON_BOOLEAN,
	JSON_NULL,
} JSON_TYPES;

typedef struct {
	union {
		apr_hash_t *object;
		apr_array_header_t *array;
		double dnumber;
		long lnumber;
		char *string;
		char boolean;
	} value;
	JSON_TYPES type;
} json_value;

json_value * decode_json(const char *injson,apr_pool_t *mpool);
void encode_json(json_value *json);

/** HELPER FUNCTION **/ 
json_value* json_create_null(apr_pool_t *mpool);
json_value* json_create_long(apr_pool_t *mpool,long number);
json_value* json_create_double(apr_pool_t *mpool,double number);
json_value* json_create_string(apr_pool_t *mpool,const char *string);
json_value* json_create_boolean(apr_pool_t *mpool,char b);
json_value* json_create_object(apr_pool_t *mpool);
json_value* json_create_array(apr_pool_t *mpool,int size);
void  json_add_object(json_value *jobject, const char *key, json_value *value); 
void  json_append_array(json_value *jarray,  json_value *value); 
char* json_serialize(apr_pool_t *mpool, json_value *json);
apr_status_t json_get_string(json_value *injson, const char * key, json_value **value);
apr_status_t json_get_sql(json_value *injson, json_value **sql);
apr_status_t json_get_cache_ttl(json_value *injson, long *cache_ttl );
int json_allows_caching(json_value *json);
apr_status_t json_get_cache_ttl(json_value *injson, long *cache_ttl );
#define JSON_KEY_SQL "SQL"
#define JSON_KEY_CACHE "CACHE"

#endif //_SIMPLEJSON_H_
