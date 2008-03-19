#include "simplejson.h"

/* $Id: jsonhelper.c,v 1.3 2008/02/29 00:18:51 derek Exp $ */

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
	out->value.object = json_skip_create(mpool,7,(json_skip_cmp_t)strcmp);
	return out;
}
json_value* json_array_create(apr_pool_t *mpool,int asize) {
	json_value *out = apr_palloc(mpool,sizeof(json_value));
	out->type = JSON_ARRAY;
	out->value.array = apr_array_make(mpool,asize,sizeof(json_value *));
	return out;
}
void  json_object_add(json_value *jobject, const char *key, json_value *value) {
	json_skip_put(jobject->value.object,(char*)key,value);
}
void  json_array_append(json_value *jarray,  json_value *value) {
	*((json_value**)(apr_array_push(jarray->value.array))) = value;
}

