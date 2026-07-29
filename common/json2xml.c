#include "json2xml.h"
#include <apr_buckets.h>

/* $Id: json2xml.c,v 1.1 2008/03/07 16:33:05 derek Exp $ */

static apr_status_t xml_object_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*, json_value *json);
static apr_status_t xml_array_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*,json_value *json);
static apr_status_t xml_boolean_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*,json_value *json);
static apr_status_t xml_string_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*,json_value *json);
static apr_status_t xml_null_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*,json_value *json);
static apr_status_t xml_number_serialize(apr_bucket_brigade *bbrigade, apr_pool_t*,json_value *json);
static apr_status_t xml_serialize_internal(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,  json_value *json);

apr_status_t xml_array_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool, json_value *json){ 
	if(json->value.array->nelts == 0) { return apr_brigade_printf(bbrigade,NULL,NULL,"[]"); }
	int i;
	apr_brigade_printf(bbrigade,NULL,NULL,"<array>");
	for(i = 0; i < json->value.array->nelts; i++) { 
			apr_brigade_printf(bbrigade,NULL,NULL,"<item>");
			json_value *v = *((json_value**)(json->value.array->elts + (json->value.array->elt_size * i)));
			xml_serialize_internal(bbrigade,mpool,v);
			apr_brigade_printf(bbrigade,NULL,NULL,"</item>");
	}
	return apr_brigade_printf(bbrigade,NULL,NULL,"</array>");
}

apr_status_t xml_object_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,json_value *json) { 
	apr_status_t status;
	if(json->value.object->node_count) {
					json_skip_node_t *list = json->value.object->node->next_list[0];
					status  = apr_brigade_printf(bbrigade,NULL,NULL,"<object>");
					if(list) {
									do{
													const char *k = (const char*) list->key;	
													json_value *v = (json_value *)list->data;
													json_value key;
													key.type = JSON_STRING;
													key.value.string = (char *)k;
													status = apr_brigade_printf(bbrigade,NULL,NULL,"<key>");
													status = xml_string_serialize(bbrigade,mpool,&key);
													status = apr_brigade_printf(bbrigade,NULL,NULL,"</key>");
													status = apr_brigade_printf(bbrigade,NULL,NULL,"<value>");
													status = xml_serialize_internal(bbrigade,mpool,(json_value*)v);
													status = apr_brigade_printf(bbrigade,NULL,NULL,"</value>");
													list = list->next_list[0];
									}while(list!=NULL);
					}
				status = apr_brigade_printf(bbrigade,NULL,NULL,"</object>");
	} else {
				status = apr_brigade_printf(bbrigade,NULL,NULL,"<object></object>");
	}
	return status;
}
apr_status_t  xml_string_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,json_value *json) { 
	static const char hex[] = "0123456789ABCDEF";
	//worst case is a 6 byte &#xNN; reference per input byte, +1 terminator, +2 slack
	char *out = apr_pcalloc( mpool, sizeof(char) * ((strlen(json->value.string)*6) +3));
	unsigned char *p = (unsigned char*)json->value.string;
	char *op = out;
	while(*p !='\0') { 
		switch(*p) { 
			case '&': *op++ = '&'; *op++='a'; *op++='m'; *op++='p'; *op++=';'; break;
			case '<': *op++ = '&'; *op++='l'; *op++='t';  *op++=';'; break;
			case '>': *op++ = '&'; *op++='g'; *op++='t';  *op++=';'; break;
			case '\t':
			case '\n':
			case '\r':
				//legal in XML 1.0, but emit as a reference so they survive whitespace normalisation
				*op++='&'; *op++='#'; *op++='x';
				*op++=hex[(*p >> 4) & 0x0f]; *op++=hex[*p & 0x0f]; *op++=';';
				break;
			default:
					//XML 1.0 cannot represent the remaining C0 controls in any form - drop them
					if(*p < 0x20) break;
					*op++= *p;
		}
		p++;
	}
	return apr_brigade_write(bbrigade,NULL,NULL,out,op-out);
}
apr_status_t xml_number_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,  json_value *json) {
	if(json->type == JSON_LONG) {
		return apr_brigade_printf(bbrigade,NULL,NULL,"%ld",json->value.lnumber);
	} else if(json->type == JSON_LONGLONG) {
		return apr_brigade_printf(bbrigade,NULL,NULL,"%lld",json->value.llnumber);
	} else {
		char *buf = apr_palloc(mpool,sizeof(char)*512);
		snprintf(buf,512,"%g",json->value.dnumber);
		//apr %g doesn't prepend leading 0 for values less than 1 - violates json parsers
		return apr_brigade_printf(bbrigade,NULL,NULL,buf);
	}
}

apr_status_t xml_boolean_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,json_value *json) { 
	return apr_brigade_printf(bbrigade, NULL,NULL, json->value.boolean? "true" : "false");
}
apr_status_t xml_null_serialize(apr_bucket_brigade *bbrigade, apr_pool_t *mpool, json_value *json) { 
	return apr_brigade_printf(bbrigade, NULL,NULL, "null");
}
apr_status_t xml_serialize_internal(apr_bucket_brigade *bbrigade, apr_pool_t *mpool,  json_value *json) { 
	switch(json->type){
		case JSON_STRING: return xml_string_serialize(bbrigade,mpool,json);
		case JSON_LONG:
		case JSON_LONGLONG:
		case JSON_DOUBLE: return xml_number_serialize(bbrigade,mpool,json);
		case JSON_BOOLEAN: return xml_boolean_serialize(bbrigade,mpool,json); 
		case JSON_NULL: return xml_null_serialize(bbrigade,mpool,json); 
		case JSON_OBJECT: return xml_object_serialize(bbrigade,mpool,json); 
		case JSON_ARRAY: return xml_array_serialize(bbrigade,mpool,json); 
	}
	return 0; // huh? mr. compiler
}

char * xml_serialize(apr_pool_t *mpool, json_value *json) { 
	apr_off_t bl;
  apr_bucket_alloc_t *bucket_allocator = apr_bucket_alloc_create(mpool);
  apr_bucket_brigade*  bbrigade = apr_brigade_create(mpool, bucket_allocator);
	xml_serialize_internal(bbrigade,mpool,json);
  apr_brigade_length(bbrigade,0,&bl);
 
  char *out = apr_pcalloc(mpool,bl+1);
  apr_size_t len = bl;
  apr_brigade_flatten(bbrigade,out,&len);
	return out;
}
