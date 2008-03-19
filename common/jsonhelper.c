#include "simplejson.h"

/* $Id: jsonhelper.c,v 1.2 2007/05/09 20:55:00 derek Exp $ */

/** HELPER FUNCTION **/ 
json_value* json_null_create(apr_pool_t *mpool) {
	json_value *out = apr_palloc(mpool,sizeof(json_value));
	out->type = JSON_NULL;
	return out;
}
json_value* json_long_create(apr_pool_t *mpool,long number) {
	json_value *out = apr_palloc(mpool,sizeof(json_value));
	out->type = JSON_LONG;
	out->value.lnumber = number;
	return out;
}
json_value* json_double_create(apr_pool_t *mpool,double number){
	json_value *out = apr_palloc(mpool,sizeof(json_value));
	out->type = JSON_DOUBLE;
	out->value.dnumber = number;
	return out;
}
json_value* json_string_create(apr_pool_t *mpool,const char *string) {
	json_value *out = apr_palloc(mpool,sizeof(json_value));
	out->type = JSON_STRING;
	out->value.string = apr_pstrdup(mpool,string);
	return out;
}
json_value* json_boolean_create(apr_pool_t *mpool,char b){
	json_value *out = apr_palloc(mpool,sizeof(json_value));
	out->type = JSON_BOOLEAN;
	out->value.boolean = b;
	return out;
}
json_value* json_object_create(apr_pool_t *mpool){
	json_value *out = apr_palloc(mpool,sizeof(json_value));
	out->type = JSON_OBJECT;
	out->value.object = apr_hash_make(mpool);
	return out;
}
json_value* json_array_create(apr_pool_t *mpool,int asize) {
	json_value *out = apr_palloc(mpool,sizeof(json_value));
	out->type = JSON_ARRAY;
	out->value.array = apr_array_make(mpool,asize,sizeof(json_value *));
	return out;
}
void  json_object_add(json_value *jobject, const char *key, json_value *value) {
	apr_hash_set(jobject->value.object,key,APR_HASH_KEY_STRING,value);
}
void  json_array_append(json_value *jarray,  json_value *value) {
	*((json_value**)(apr_array_push(jarray->value.array))) = value;
}

