#include "dbslayer_logging.h"

/* $Id: dbslayer_logging.c,v 1.4 2007/05/09 20:55:00 derek Exp $ */

int dbslayer_log_open(dbslayer_log_manager_t **_manager,const char *filename, int nentries, apr_pool_t *mpool){
	apr_status_t status;
	apr_pool_t *logpool;
	apr_pool_create(&logpool,mpool);
	dbslayer_log_manager_t *manager = *_manager = apr_pcalloc(logpool,sizeof(dbslayer_log_manager_t));
	manager->mpool = logpool;


	//only create if there is a non-null filename passed in
	if(filename) { 
		status = apr_thread_mutex_create(&manager->file_mutex,APR_THREAD_MUTEX_DEFAULT,manager->mpool);
		status = apr_file_open(&manager->fhandle,filename,APR_CREATE | APR_WRITE | APR_APPEND | APR_BUFFERED,APR_OS_DEFAULT,manager->mpool);
	}
	status = apr_thread_mutex_create(&manager->list_mutex,APR_THREAD_MUTEX_DEFAULT,manager->mpool);

	manager->nentries = nentries;
	manager->entries = apr_pcalloc(manager->mpool,sizeof(query_entry_t)*nentries);
	return status;
}

int dbslayer_log_message(dbslayer_log_manager_t *manager, const char *message) {
	if(message) {  
		apr_thread_mutex_lock(manager->file_mutex);
		apr_size_t nbytes =  strlen(message);
		apr_file_write(manager->fhandle,message,&nbytes);
		apr_thread_mutex_unlock(manager->file_mutex);
	}
	return 0;
}
int dbslayer_log_close(dbslayer_log_manager_t *manager) { 
	if(manager->fhandle) {
		apr_thread_mutex_destroy(manager->file_mutex);
		apr_file_close(manager->fhandle);
		apr_pool_destroy(manager->mpool);
	}
	return 0;
}
int dbslayer_log_request(dbslayer_log_manager_t *manager, apr_pool_t *mpool, apr_socket_t *conn, 
		const char *request_line, int response_code, int nbytes_sent, apr_int64_t time_toservice) { 
	//generate data
	char dstring[1024];
	apr_int64_t current_time = apr_time_now();
	apr_size_t result_size;
	apr_time_exp_t ltime;
	apr_time_exp_lt(&ltime,current_time);
	apr_strftime (dstring, &result_size, sizeof(dstring), "%d/%b/%Y:%H:%M:%S %z", &ltime );

	apr_sockaddr_t *client_addr;
	char *client_ip;
	apr_socket_addr_get(&client_addr,0,conn);
	apr_sockaddr_ip_get(&client_ip,client_addr);
	if(manager->fhandle){
		char *message = apr_pstrcat(mpool,client_ip," - - ","[",dstring,"] \"",request_line,"\" ",
				apr_itoa(mpool,response_code)," ",apr_itoa(mpool,nbytes_sent), " ",apr_ltoa(mpool,time_toservice), "\n",NULL);
		dbslayer_log_message(manager,message);
	}
	dbslayer_log_add_entry(manager,mpool,client_ip,current_time,request_line,response_code,nbytes_sent,time_toservice);
	return 0;
}
int dbslayer_log_err_message(dbslayer_log_manager_t *manager,apr_pool_t *mpool,apr_socket_t *conn, 
		const char *request_line, const char * error_message) { 

	//generate data
	char dstring[1024];
	apr_size_t result_size;
	apr_int64_t current_time = apr_time_now();
	apr_time_exp_t ltime;
	apr_time_exp_lt(&ltime,current_time);
	apr_strftime (dstring, &result_size, sizeof(dstring), "%d/%b/%Y:%H:%M:%S %z", &ltime );

	apr_sockaddr_t *client_addr;
	char *client_ip;
	apr_socket_addr_get(&client_addr,0,conn);
	apr_sockaddr_ip_get(&client_ip,client_addr);

	if (manager->fhandle){
		char *message = apr_pstrcat(mpool,client_ip," - - ","[",dstring,"] \"",request_line,"\" ",
			error_message, "\n",NULL);
		dbslayer_log_message(manager,message);
	}
	dbslayer_log_add_error(manager, mpool,client_ip,current_time,request_line, error_message);
	return 0;
}


void dbslayer_log_add_entry(dbslayer_log_manager_t *manager, apr_pool_t *mpool,
			const char *client_ip,apr_int64_t rtime,
			const char *request_line,int response_code, 
			int nbytes_sent, apr_int64_t time_toservice ) { 

	json_value *container = json_object_create(mpool);
	json_object_add(container,"client_ip",json_string_create(mpool,client_ip));
	json_object_add(container,"request_time",json_long_create(mpool,rtime / (1000*1000)));
	json_object_add(container,"request",json_string_create(mpool,request_line));
	json_object_add(container,"response_code",json_long_create(mpool,response_code));
	json_object_add(container,"bytes_sent",json_long_create(mpool,nbytes_sent));
	json_object_add(container,"time_toservice",json_long_create(mpool,time_toservice));
	char *json_entry = strdup(json_serialize(mpool,container)); //we want our own copy of this data

	//smallest chunk in the mutex
	apr_thread_mutex_lock(manager->list_mutex);
	manager->offset++;
	if(manager->offset == manager->nentries)  manager->offset = 0;
	free(manager->entries[manager->offset].json_view);
	manager->entries[manager->offset].json_view = json_entry;
	apr_thread_mutex_unlock(manager->list_mutex);
}

void dbslayer_log_add_error(dbslayer_log_manager_t *manager, apr_pool_t *mpool,
			const char *client_ip,apr_int64_t rtime,
			const char *request_line, const char *error_msg ) { 

	json_value *container = json_object_create(mpool);
	json_object_add(container,"client_ip",json_string_create(mpool,client_ip));
	json_object_add(container,"request_time",json_long_create(mpool,rtime / (1000*1000)));
	json_object_add(container,"request",json_string_create(mpool,request_line));
	json_object_add(container,"error",json_string_create(mpool,error_msg));
	char *json_entry = strdup(json_serialize(mpool,container)); //we want our own copy of this data
	//smallest chunk in the mutex
	apr_thread_mutex_lock(manager->list_mutex);
	manager->offset++;
	if(manager->offset == manager->nentries)  manager->offset = 0;
	free(manager->entries[manager->offset].json_view);
	manager->entries[manager->offset].json_view = json_entry;
	apr_thread_mutex_unlock(manager->list_mutex);
}

char * dbslayer_log_get_entries(dbslayer_log_manager_t *manager, apr_pool_t *mpool) { 
	char *json_str = "[";
	int i;
	apr_thread_mutex_lock(manager->list_mutex);
	for(i=manager->offset+1;i < manager->nentries; i++) { 
		if(manager->entries[i].json_view !=NULL) { 
			json_str = apr_pstrcat(mpool, json_str, strcmp(json_str,"[")==0 ? "" : ",",manager->entries[i].json_view,NULL);
		}
	} 	
	for(i=0;i <manager->offset+1; i++) { 
		if(manager->entries[i].json_view !=NULL) { 
			json_str = apr_pstrcat(mpool, json_str,strcmp(json_str,"[")==0 ? "" : ",",manager->entries[i].json_view,NULL);
		}
	} 	
	json_str = apr_pstrcat(mpool,json_str,"]",NULL);
	apr_thread_mutex_unlock(manager->list_mutex);
	return json_str;
}
