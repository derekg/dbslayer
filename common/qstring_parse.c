#include <apr_general.h>
#include <apr_network_io.h>
#include <apr_strings.h>
#include <apr_uri.h>
#include <apr_thread_rwlock.h>

#include "slayer_util.h"

json_skip_head_t * parse_qstring(apr_pool_t *mpool,const char *qstring) { 
	json_skip_head_t *result = json_skip_create(mpool,4,(json_skip_cmp_t)strcmp);
	if(qstring) {
		int ct = 0;
		char *m;
		char **params = malloc(strlen(qstring)*sizeof(char*)); 
		char *q = apr_pstrdup(mpool,qstring);
		char *t =  apr_strtok(q,"&",&m);
		while(t !=NULL) {
			params[ct] = t;
			ct++; 
			t = apr_strtok(NULL,"&",&m);
		}
		int i = 0; 
		for(i = 0; i < ct; i++) { 
			char *x = params[i];
			char *name = NULL; 
			char *value = NULL;
			name = apr_strtok(x,"=",&value);	
			if(name	&& value){
				json_skip_put(result,name,urldecode(mpool,value));	
			}
		}
		free(params);
	}
	return result;
}

