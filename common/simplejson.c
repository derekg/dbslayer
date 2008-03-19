#include "simplejson.h"
#include "slayer_utf8.h"

/* $Id: simplejson.c,v 1.6 2008/02/29 00:18:51 derek Exp $ */

/** create json_value objects **/


/** decode json from a string **/
json_value* decode_json_array(json_string *injson);
json_value* decode_json_boolean(json_string *injson);
json_value * decode_json_null(json_string *injson);
json_value* decode_json_number(json_string *injson);
json_value* decode_json_object(json_string *injson);
json_value* decode_json_string(json_string *injson);
json_value * decode_json_value(json_string *injson); 

/** dump more than encode **/
void encode_json_object(json_value *json);
void encode_json_array(json_value *json);
void encode_json_boolean(json_value *json);
void encode_json_string(json_value *json);
void encode_json_null(json_value *json);
void encode_json_number(json_value *json);

typedef struct _json_link_t {
	json_value *value;
	struct _json_link_t *next;
} json_link_t;

json_value* decode_json_array(json_string *injson) { 
	if(injson == NULL || injson->offset == NULL || injson->offset >= injson->end) return NULL;

	apr_pool_t *link_pool;
	
	json_link_t *head,*node;
	int link_count = 0;
	head = node = NULL;
	
	/*
	json_value *out = apr_palloc(injson->mpool,sizeof(json_value));
	out->value.array = apr_array_make(injson->mpool,100,sizeof(json_value *));
	out->type = JSON_ARRAY;
	*/

	injson->offset++; // toss of the leading [
	while(injson->offset < injson->end && isspace(*injson->offset)) injson->offset++;
	if(injson->offset >= injson->end) return NULL;
	if(*(injson->offset) == ']') {
		injson->offset++;
		return json_array_create(injson->mpool,1);
	}
		
	json_value *element = decode_json_value(injson);
	if(element==NULL) return NULL;
	apr_pool_create(&link_pool,NULL);
	head = node = apr_pcalloc(link_pool,sizeof(json_link_t));
	node->value = element;
	link_count++;
	//*((json_value**)(apr_array_push(out->value.array))) = element;
	
	while( injson->offset < injson->end && *(injson->offset)!=']') { 
		while(injson->offset < injson->end && isspace(*injson->offset)) injson->offset++;
		if(injson->offset >= injson->end) return NULL;
		switch(*(injson->offset)) { 
			case ']':
					goto array_closure;
			case ',':
					injson->offset++;
					if((element = decode_json_value(injson))==NULL) {
						apr_pool_destroy(link_pool);
						return NULL;
					}
					//*((json_value**)(apr_array_push(out->value.array))) = element;
					node->next = apr_pcalloc(link_pool,sizeof(json_link_t));
					node->next->value = element;
					node = node->next;
					link_count++;
					break;
			default:
					apr_pool_destroy(link_pool);
					return NULL;
		}
	}
array_closure:
		if(injson->offset >= injson->end || *(injson->offset) !=']'){
			apr_pool_destroy(link_pool);
			 return NULL;
		}
		injson->offset++; //chew up trailing ']'
		json_value *out = apr_palloc(injson->mpool,sizeof(json_value));
		out->value.array = apr_array_make(injson->mpool,link_count,sizeof(json_value *));
		out->type = JSON_ARRAY;
		node = head;
		while(node) { 
			*((json_value**)(apr_array_push(out->value.array))) = node->value;
			node = node->next;	
		}
		apr_pool_destroy(link_pool);
	return out;
}


json_value* decode_json_object(json_string *injson){

	if(injson == NULL || injson->offset == NULL) return NULL;

	json_value *out = apr_palloc(injson->mpool,sizeof(json_value));
	out->value.object = json_skip_create(injson->mpool,7,(json_skip_cmp_t)strcmp);
	out->type = JSON_OBJECT;

	injson->offset++; // toss of the leading  {
	while(isspace(*injson->offset)) injson->offset++;
	//empty { }
	if(*(injson->offset) == '}') {
		injson->offset++;
		return out;
	}

	//PULL OUT THE FIRST VALUE
	while(injson->offset < injson->end && isspace(*injson->offset)) injson->offset++;
	if(injson->offset >= injson->end || *(injson->offset) != '"') return NULL;
	json_value *name = decode_json_string(injson);
	if(name == NULL) return NULL;
	while(injson->offset < injson->end && isspace(*injson->offset)) injson->offset++;
	if(injson->offset >= injson->end || *(injson->offset) != ':') return NULL;
	injson->offset++; //eat the :
	while(injson->offset < injson->end && isspace(*injson->offset)) injson->offset++;
	json_value *value = decode_json_value(injson);
	if(value == NULL) return NULL;
	json_skip_put(out->value.object,name->value.string,value);

	while( injson->offset < injson->end && *(injson->offset) != '}') { 
		while( injson->offset < injson->end && isspace(*injson->offset)) injson->offset++;
		if(injson->offset < injson->end && *(injson->offset) == '}')  {
			goto object_closure;
		} else if (injson->offset < injson->end &&*(injson->offset) == ',') { 
			injson->offset++; // eat the comma
			while(injson->offset < injson->end && isspace(*injson->offset)) injson->offset++;
			if(injson->offset >= injson->end || *(injson->offset) != '"') return NULL;
			json_value *name = decode_json_string(injson);
			if(name == NULL) return NULL;
			while(injson->offset < injson->end &&isspace(*injson->offset)) injson->offset++;
			if(injson->offset >= injson->end ||*(injson->offset) != ':') return NULL;
			injson->offset++;
			while(injson->offset < injson->end && isspace(*injson->offset)) injson->offset++;
			json_value *value = decode_json_value(injson);
			if(value == NULL) return NULL;
			json_skip_put(out->value.object,name->value.string,value);
		} else {
			return NULL;
		}
	}
object_closure:
		if(injson->offset >= injson->end ||*(injson->offset) !='}') return NULL;
		injson->offset++; //chew up trailing '}'
	return out;
}

json_value* decode_json_string(json_string *injson) {
	json_value *out = NULL;
	const char *ptr;
	injson->offset++; //advance past the \"
	ptr = injson->offset;
	while( ptr< injson->end && *ptr !='"') { 
			if(*ptr == '\\') { 
							ptr++;
							switch(*ptr) { 
								case 'u':  
								case '\\':
								case '/':
								case 'n':
								case 'r':
								case 't':
								case 'f':
								case 'b':
								case '"':
									break;
								default:
									return NULL;
							}
			}
			ptr++;
	}
	//make sure the string is terminated w/ "
	if(ptr >= injson->end) return NULL;

	int vsize  = (ptr - injson->offset) + 1;
	out = apr_palloc(injson->mpool,sizeof(json_value));
	out->value.string = apr_palloc(injson->mpool,vsize);
	memset(out->value.string,0,vsize);
	out->type = JSON_STRING;
	char * optr =  out->value.string;
	while( injson->offset <  injson->end && *(injson->offset) !='"') { 
		if(*(injson->offset) == '\\') { 
			injson->offset++;
			if(injson->offset >= injson->end )  { return NULL; }
			switch(*(injson->offset)) { 
				case 'u':  // we are not excepting this for now - use UTF-8
					/** THIS IS REQUIRED TO A 4 DIGIT HEX NUMBER **/
					{
						if(strlen(injson->offset+1)<4) return NULL;
						char code[5];
						code[4] = '\0';
						memcpy(code,injson->offset+1,4); //copy 4 digit
						int value = slayer_hex2int(code);
						if(value == -1) return NULL; //contained something other than 0-9A-Fa-f
						char *encode = slayer_escaped2utf8(injson->mpool,value);
						if(encode == NULL) return NULL; //not a valid range
						while( *encode!='\0') *optr++ = *encode++;
						injson->offset+=4; // it will get the U at the end
					}
					break;
				case '\\':
					*optr =  '\\'; optr++; 
					break;
				case '/':
					*optr =  '/'; optr++;
					break;
				case 'n':
					*optr =  '\n'; optr++;
					break;
				case 'r':
					*optr =  '\r'; optr++;
					break;
				case 't':
					*optr =  '\t'; optr++;
					break;
				case 'f':
					*optr =  '\f'; optr++;
					break;
				case 'b':
					*optr =  '\b'; optr++;
					break;
				case '"':
					*optr =  '"'; optr++;
					break;
				default:
					//should be impossible
					return NULL;
			}
		} else {
			*optr = *(injson->offset);
			optr++;
		}
		injson->offset++;
	}
	if(injson->offset >= injson->end ||*(injson->offset) != '"')  { return NULL; } 
	injson->offset++;
	return out;
}
json_value* decode_json_boolean(json_string *injson){
	json_value *out = NULL;
	if(injson->end - injson->offset > 3  && strncmp("true",injson->offset,4) == 0) { 
		out = apr_palloc(injson->mpool,sizeof(json_value));
		out->value.boolean = 1;
		out->type = JSON_BOOLEAN;
		injson->offset +=4;
	} else if (injson->end - injson->offset > 4 && strncmp("false",injson->offset,5)==0){
		out = apr_palloc(injson->mpool,sizeof(json_value));
		out->value.boolean = 0;
		out->type = JSON_BOOLEAN;
		injson->offset +=5;
	}
	return out;
}

json_value* decode_json_number(json_string *injson){
	json_value *out = NULL;
	char *optr = (char*)injson->end;
	const char *pptr = injson->offset;
	char isfloat = 0;
	while( pptr != NULL && pptr < injson->end) {  
		if(isdigit(*pptr) || (*pptr == '-' && pptr == injson->offset)) { 
			pptr++;
		}else { 
			switch(*pptr) {
				case 'e':
				case 'E':
				case '.':
					isfloat = 1;
				default:
					pptr = NULL;
					break;
			}
		}
	}
	if(isfloat) { 
		double value = strtod(injson->offset,&optr);
		if( injson->offset != optr) { 
			out = apr_palloc(injson->mpool,sizeof(json_value));
			out->value.dnumber= value;
			out->type  = JSON_DOUBLE;
			injson->offset = optr;
		}	
	} else {
		long value = strtol(injson->offset,&optr,10);
		if( injson->offset != optr) { 
			out = apr_palloc(injson->mpool,sizeof(json_value));
			out->value.lnumber = value;
			out->type  = JSON_LONG;
			injson->offset = optr;
		}	
	}
	return out;
}

json_value * decode_json_null(json_string *injson) { 
	json_value *out = NULL;
	if(injson->end - injson->offset > 3 && strncmp(injson->offset,"null",4) == 0) { 
		out = apr_palloc(injson->mpool,sizeof(json_value));
		out->type = JSON_NULL;
		injson->offset +=4;
	}
	return out;
}

json_value * decode_json_value(json_string *injson) { 
	json_value *outjson= NULL;	
	if(injson->jstring ) {
		while(isspace(*(injson->offset)))  injson->offset++;
		switch(*(injson->offset)) { 
						case '"':  outjson = decode_json_string(injson); break;
						case '[':  outjson = decode_json_array(injson); break;
						case '{':  outjson = decode_json_object(injson); break;
						case 'n':  outjson = decode_json_null(injson); break;
						case 't':  
						case 'f':  outjson = decode_json_boolean(injson); break;
						case '-':  
						case '1':  
						case '2':  
						case '3':  
						case '4':  
						case '5':  
						case '6':  
						case '7':  
						case '8':  
						case '9':  
						case '0':  outjson = decode_json_number(injson); break;
						default:
								printf("XError %s\n",injson->jstring); return NULL;
		}
	}//end of if
	return outjson;
}

json_value * decode_json(const char *injson,int injson_size, apr_pool_t *mpool) { 
	json_string jstring;
	jstring.jstring = injson;
	jstring.end = injson + injson_size;
	jstring.offset = injson;
	jstring.mpool = mpool;

	json_value *out = decode_json_value(&jstring);

	//chew up trailing ws - and check for crap at the end
	while(jstring.offset < jstring.end && isspace(*(jstring.offset))) jstring.offset++;
	if(jstring.offset != jstring.end ) { out = NULL; }
	return out;
}



void encode_json_array(json_value *json) { 
	printf("[");
	int i;
	for(i = 0; i < json->value.array->nelts; i++) { 
			if(i > 0) printf(" , ");
			json_value *v = *((json_value**)(json->value.array->elts + (json->value.array->elt_size * i)));
			encode_json(v);
	}
	printf("]");
}

void encode_json_object(json_value *json) { 
	printf("{");
	if(json->value.object->node_count) {
                json_skip_node_t *list = json->value.object->node->next_list[0];
                if(list) {
                        do{
				const char *k = (const char*) list->key;	
                                json_value *v = (json_value *)list->data;
				json_value key;
				key.type = JSON_STRING;
				key.value.string = (char *)k;
				encode_json_string(&key);
				printf(" : ");
				encode_json(v);
                                list = list->next_list[0];
				if(list) printf(",");
                        }while(list!=NULL);
                }
	}
	printf("}");
}
void encode_json_string(json_value *json) { 
	char *p = json->value.string;
	printf("\"");
	while(*p !='\0') { 
		switch(*p) { 
			case '\n': printf("\\n"); break;
			case '\r': printf("\\r"); break;
			case '\t': printf("\\t"); break;
			case '\b': printf("\\b"); break;
			case '\f': printf("\\f"); break;
			case '\\': printf("\\\\"); break;
			case '/': printf("\\/"); break;
			case '"': printf("\""); break;
			default:
					printf("%c",*p);
		}
		p++;
	}
	printf("\"");
}
void encode_json_number(json_value *json) { 
	if(json->type == JSON_LONG) {
		printf("%ld",json->value.lnumber);
	} else {
		printf("%g",json->value.dnumber);
	}
}
void encode_json_boolean(json_value *json) { 
	printf("%s",json->value.boolean? "true" : "false");
}
void encode_json_null(json_value *json) { 
	printf("null");
}
void encode_json(json_value *json) { 
	switch(json->type){
		case JSON_STRING: encode_json_string(json); break;
		case JSON_LONG: 
		case JSON_DOUBLE: encode_json_number(json); break;
		case JSON_BOOLEAN: encode_json_boolean(json); break;
		case JSON_NULL: encode_json_null(json); break;
		case JSON_OBJECT: encode_json_object(json); break;
		case JSON_ARRAY: encode_json_array(json); break;
	}
}

#ifdef SIMPLEJSON_MAIN
int main(int argc, char **argv) { 
	apr_pool_t *mpool;
	apr_pool_initialize();
	apr_pool_create(&mpool,NULL);
	json_value *out = decode_json(argv[1],mpool);
	if(out) { 
		printf("got a value\n");
		encode_json(out);
		printf("\n");
	}
	apr_pool_destroy(mpool);
	apr_pool_terminate();
	return 0;
}
#endif
