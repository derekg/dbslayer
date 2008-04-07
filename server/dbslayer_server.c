#include "slayer_http_server.h"
#include "dbaccess.h"
#include "simplejson.h"
#include "slayer_util.h"
#include "json2xml.h"

#include <apr_portable.h>

typedef struct _dbslayer_config_t { 
	char *server;
	char *multiserver;
	char *configure;
	char *username;
	char *password;
} dbslayer_config_t;

void * db_global_init(apr_pool_t *pmpool, int argc, char **argv) {
	int i = 0;
	mysql_library_init(0,NULL,NULL); //setup init
	dbslayer_config_t *config = apr_pcalloc(pmpool,sizeof(dbslayer_config_t));
	//looking for -> -s server -c config -u username -x password
	for( i = 0; i < argc; i++) { 
		if( i+1 < argc && argv[i][0] == '-' ) { 
			switch(argv[i][1]) { 
				case 's': config->server = apr_pstrdup(pmpool,argv[i+1]); break;
				case 'c': config->configure = apr_pstrdup(pmpool,argv[i+1]); break;
				case 'u': config->username = apr_pstrdup(pmpool,argv[i+1]); break;
				case 'x': config->password = apr_pstrdup(pmpool,argv[i+1]); break;
				case 'm': config->multiserver= apr_pstrdup(pmpool,argv[i+1]); break;
			}			
		}
	}
	if((config->server == NULL && config->multiserver == NULL) || config->configure == NULL ) { 
		fprintf(stderr,"Failed to configure dbslayer - [-s server[:server] or -m server[:server] ] and -c configure are required arguments\n");
		exit(0);
	}
	return config;
}
void  db_global_destroy(void *_config) {
	mysql_library_end();	
}
void * db_thread_init(apr_pool_t *mpool, void *_global_config) {
	dbslayer_config_t *config = (dbslayer_config_t*) _global_config;
	unsigned int id =  apr_os_thread_current();
	return db_handle_init(config->username,config->password,config->multiserver == NULL ? config->server : config->multiserver,config->configure,&id,config->multiserver == NULL ? 0 : 1);
}
void  db_thread_destroy(void *x) { 
	db_handle_destroy((db_handle_t*)x);
}

void *dbjson_handler(slayer_http_server_t *server, void *_global_config, slayer_http_connection_t *client, void *local_config) {
	db_handle_t *dbhandle = (db_handle_t*)local_config;
	char *output = "{\"ERROR\" : \"we got problems -  couldn't parse your incoming json\"}";
	json_value *stmt = NULL;
	json_value *output_format = NULL;
	char *content_type = SLAYER_MT_TEXT_PLAIN;

	if(strcmp(client->request->uri.path,"/dbform") == 0) { 	
					json_skip_head_t *form =  parse_qstring(client->request->mpool,client->request->uri.query);	
					stmt  = json_object_create(client->request->mpool);
					if(form->node_count) {
						json_skip_node_t *list = form->node->next_list[0];
						while(list !=NULL) {
								const char *k = (const char*)list->key;	
								const char *v = (const char*)list->data;
								json_object_add(stmt,k,json_string_create(client->request->mpool,v));
								list = list->next_list[0];
						}
					}
	} else { 
					char *cquery = urldecode(client->request->mpool,client->request->uri.query);
					stmt = decode_json(cquery,strlen(cquery),client->request->mpool); 
	}

	if(dbhandle && stmt) {
					output_format =  json_skip_get(stmt->value.object,"FORMAT");
					json_value *result = dbexecute(dbhandle,stmt,client->request->mpool);
					json_value *errors = json_skip_get(result->value.object,"MYSQL_ERROR");	
					if(errors) { 
									char *http_request = apr_pstrcat(client->request->mpool,client->request->parse->method == HTTP_METHOD_GET ? "GET ": "POST ",client->request->parse->uri_data,client->request->parse->version == HTTP_10 ? " HTTP/1.0" : " HTTP/1.0",NULL);
									slayer_server_log_err_message(server->elmanager,client->request->mpool,client->conn,http_request,apr_pstrcat(client->request->mpool,"ERROR: ",errors->value.string,NULL));
					}
					if(output_format && output_format->type == JSON_STRING && strcmp(output_format->value.string,"xml")==0) { 
						output = xml_serialize(client->request->mpool,result);
						content_type = "text/xml";
					} else {
						output = json_serialize(client->request->mpool,result);
					}
	} else  {
					char *http_request = apr_pstrcat(client->request->mpool,client->request->parse->method == HTTP_METHOD_GET ? "GET ": "POST ",client->request->parse->uri_data,client->request->parse->version == HTTP_10 ? " HTTP/1.0" : " HTTP/1.0",NULL);
					slayer_server_log_err_message(server->elmanager,client->request->mpool,client->conn,http_request,"ERROR: Couldn't parse incoming JSON");
	}

	client->request->response_code = 200;
	slayer_http_handle_response(server, client, content_type,output,-1);
	return NULL;
}

int main(int argc, char **argv) {

	slayer_http_service_map_t **service_map;
	char *urls[] = { "/db","/dbform",NULL};

	slayer_http_service_t handler;
	handler.help_string =  "-s server[:server] -c config [-u username -x password]";
	handler.service_global_init_func = db_global_init;
	handler.service_global_destroy_func = db_global_destroy;
	handler.service_thread_init_func = db_thread_init;
	handler.service_thread_destroy_func = db_thread_destroy;
	handler.service_handler_func = dbjson_handler;


	service_map = malloc(sizeof(slayer_http_service_map_t*) * 1);
	service_map[0] = malloc(sizeof(slayer_http_service_map_t));
	service_map[0]->urls = urls;
	service_map[0]->service = &handler;

/*
	slayer_http_service_t dummy_handler;
	dummy_handler.help_string = "dummy don't need to be configured";
	dummy_handler.service_global_init_func = NULL;
	dummy_handler.service_global_destroy_func = NULL;
	dummy_handler.service_thread_init_func = NULL;
	dummy_handler.service_thread_destroy_func = NULL;
	dummy_handler.service_handler_func = xdummy_handler;

	char *dummy_urls[] = {"/dummy",NULL};
	service_map[1] = malloc(sizeof(slayer_http_service_map_t));
	service_map[1]->urls = dummy_urls;
	service_map[1]->service = &dummy_handler;
*/

	return slayer_server_run(1,service_map,argc,argv,1024 * 1000 ,"dbslayer/beta-14");
}
