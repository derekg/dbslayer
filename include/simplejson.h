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
#include <apr_strings.h>
#include "json_skip.h"

typedef struct { 
	const char *jstring;
	const char *offset;
	const char *end;
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
		json_skip_head_t *object;
		apr_array_header_t *array;
		double dnumber;
		long lnumber;
		char *string;
		char boolean;
	} value;
	JSON_TYPES type;
} json_value;

//json_value * decode_json(const char *injson,apr_pool_t *mpool);
json_value * decode_json(const char *injson,int size, apr_pool_t *mpool);
void encode_json(json_value *json);

/** HELPER FUNCTION **/ 
json_value* json_null_create(apr_pool_t *mpool);
json_value* json_long_create(apr_pool_t *mpool,long number);
json_value* json_double_create(apr_pool_t *mpool,double number);
json_value* json_string_create(apr_pool_t *mpool,const char *string);
json_value* json_boolean_create(apr_pool_t *mpool,char b);
json_value* json_object_create(apr_pool_t *mpool);
json_value* json_array_create(apr_pool_t *mpool,int size);
void  json_object_add(json_value *jobject, const char *key, json_value *value); 
void  json_array_append(json_value *jarray,  json_value *value); 
char * json_serialize(apr_pool_t *mpool, json_value *json);

#endif //_SIMPLEJSON_H_
