#include "dbaccess.h"
/* $Id: dbdriver.c,v 1.4 2007/05/28 15:17:58 derek Exp $ */

int main(int argc, char **argv) {
	apr_pool_t *mpool;
	apr_pool_initialize();
	apr_pool_create(&mpool,NULL);
	db_handle_t *handle = db_handle_init(argv[1]/*user*/,argv[2]/*pass*/,argv[3]/*server*/,argv[4]/*config*/,&argc);
	json_value *query= decode_json(argv[5],mpool);
	if(query) { 
		encode_json(query);
		printf("\n");
		json_value *result = dbexecute(handle,query,mpool);
		encode_json(result);
		printf("\n");
	}
	apr_pool_destroy(mpool);
	apr_pool_terminate();
	return 0;
}
