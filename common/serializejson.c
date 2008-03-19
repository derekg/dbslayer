#include "simplejson.h"
#include <apr_buckets.h>

/* $Id: serializejson.c,v 1.8 2008/02/29 00:18:51 derek Exp $ */

static apr_status_t json_object_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*, json_value *json);
static apr_status_t json_array_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*,json_value *json);
static apr_status_t json_boolean_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*,json_value *json);
static apr_status_t json_string_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*,json_value *json);
static apr_status_t json_null_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*,json_value *json);
static apr_status_t json_number_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*,json_value *json);
static apr_status_t json_serialize_internal(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,  json_value *json);

apr_status_t json_array_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool, json_value *json){ 
	if(json->value.array->nelts == 0) { return apr_brigade_printf(bbrigade,NULL,NULL,"[]"); }
	int i;
	for(i = 0; i < json->value.array->nelts; i++) { 
			apr_brigade_printf(bbrigade,NULL,NULL,i ? " , " :  "[");
			json_value *v = *((json_value**)(json->value.array->elts + (json->value.array->elt_size * i)));
			json_serialize_internal(bbrigade,mpool,v);
	}
	return apr_brigade_printf(bbrigade,NULL,NULL,"]");
}

apr_status_t json_object_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,json_value *json) { 
	apr_status_t status;
	if(json->value.object->node_count) {
                json_skip_node_t *list = json->value.object->node->next_list[0];
		status  = apr_brigade_printf(bbrigade,NULL,NULL,"{");
                if(list) {
                        do{
				const char *k = (const char*) list->key;	
                                json_value *v = (json_value *)list->data;
				json_value key;
				key.type = JSON_STRING;
				key.value.string = (char *)k;
				status = json_string_serialize(bbrigade,mpool,&key);
				status = apr_brigade_printf(bbrigade,NULL,NULL," : ");
				status = json_serialize_internal(bbrigade,mpool,(json_value*)v);
                                list = list->next_list[0];
				status = apr_brigade_printf(bbrigade,NULL,NULL,list ? " , ": "}");
                        }while(list!=NULL);
                }
	} else{
		status = apr_brigade_printf(bbrigade,NULL,NULL,"{}");
	}
	return status;
}
apr_status_t  json_string_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,json_value *json) { 
	//length is *2 for each character if escape + 1 for terminator and +2 for leading/trailing "
	char *out = apr_pcalloc( mpool, sizeof(char) * ((strlen(json->value.string)*2) +3));  
	char *p = json->value.string;
	char *op = out;
	*op++= '"';
	while(*p !='\0') { 
		switch(*p) { 
			case '\n': *op++ = '\\'; *op++='n';break;
			case '\r': *op++ = '\\'; *op++='r';break;
			case '\t': *op++ = '\\'; *op++='t';break;
			case '\b': *op++ = '\\'; *op++='b';break;
			case '\f': *op++ = '\\'; *op++='f';break;
			case '\\': *op++ = '\\'; *op++='\\';break;
			case '/': *op++ = '\\'; *op++='/';break;
			case '"': *op++ = '\\'; *op++='"';break;
			default:
					*op++= *p;
		}
		p++;
	}
	*op++= '"';
	return apr_brigade_write(bbrigade,NULL,NULL,out,op-out);
}
apr_status_t json_number_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,  json_value *json) { 
	if(json->type == JSON_LONG) {
		return apr_brigade_printf(bbrigade,NULL,NULL,"%ld",json->value.lnumber);
	} else {  
		char *buf = apr_palloc(mpool,sizeof(char)*512);
		snprintf(buf,512,"%g",json->value.dnumber);	
		//apr %g doesn't prepend leading 0 for values less than 1 - violates json parsers
		return apr_brigade_printf(bbrigade,NULL,NULL,buf);
	}
}

apr_status_t json_boolean_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,json_value *json) { 
	return apr_brigade_printf(bbrigade, NULL,NULL, json->value.boolean? "true" : "false");
}
apr_status_t json_null_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool, json_value *json) { 
	return apr_brigade_printf(bbrigade, NULL,NULL, "null");
}
apr_status_t json_serialize_internal(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,  json_value *json) { 
	switch(json->type){
		case JSON_STRING: return json_string_serialize(bbrigade,mpool,json);
		case JSON_LONG: 
		case JSON_DOUBLE: return json_number_serialize(bbrigade,mpool,json);
		case JSON_BOOLEAN: return json_boolean_serialize(bbrigade,mpool,json); 
		case JSON_NULL: return json_null_serialize(bbrigade,mpool,json); 
		case JSON_OBJECT: return json_object_serialize(bbrigade,mpool,json); 
		case JSON_ARRAY: return json_array_serialize(bbrigade,mpool,json); 
	}
	return 0; // huh? mr. compiler
}

char * json_serialize(apr_pool_t *mpool, json_value *json) { 
	apr_off_t bl;
  apr_bucket_alloc_t *bucket_allocator = apr_bucket_alloc_create(mpool);
  apr_bucket_brigade*  bbrigade = apr_brigade_create(mpool, bucket_allocator);
	json_serialize_internal(bbrigade,mpool,json);
  apr_brigade_length(bbrigade,0,&bl);
 
  char *out = apr_pcalloc(mpool,bl+1);
  apr_size_t len = bl;
  apr_brigade_flatten(bbrigade,out,&len);
	return out;
}
