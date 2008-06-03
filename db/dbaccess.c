#include "dbaccess.h"
/* $Id: dbaccess.c,v 1.14 2008/03/06 01:50:58 derek Exp $ */

db_handle_t * db_handle_reattach(db_handle_t *handle,const char *dbserver_name) { 
	if(handle->dblookup == NULL) { 
		int ct = handle->server_offset;
		for(ct = handle->server_offset ; ct < handle->server_count; ct++) { 
			if(handle->db) mysql_close(handle->db);
			handle->db = mysql_init(NULL);
			mysql_options(handle->db,MYSQL_READ_DEFAULT_FILE,handle->config);
			mysql_options(handle->db,MYSQL_READ_DEFAULT_GROUP,handle->server[ct]);
			if(mysql_real_connect(handle->db,NULL,handle->user,handle->pass,NULL,0,NULL,CLIENT_MULTI_STATEMENTS) != NULL){
				handle->server_offset = ct;
				return handle;
			}
		}
		for(ct = 0; ct < handle->server_offset; ct++) { 
			if(handle->db) mysql_close(handle->db);
			handle->db = mysql_init(NULL);
			mysql_options(handle->db,MYSQL_READ_DEFAULT_FILE,handle->config);
			mysql_options(handle->db,MYSQL_READ_DEFAULT_GROUP,handle->server[ct]);
			if(mysql_real_connect(handle->db,NULL,handle->user,handle->pass,NULL,0,NULL,CLIENT_MULTI_STATEMENTS) != NULL){
				handle->server_offset = ct;
				return handle;
			}
		}
	} else {
		MYSQL *db = json_skip_get(handle->dblookup,(void*) dbserver_name);
		if(db) { mysql_close(db); } 
		db = mysql_init(NULL);
		mysql_options(db,MYSQL_READ_DEFAULT_FILE,handle->config);
		mysql_options(db,MYSQL_READ_DEFAULT_GROUP,dbserver_name);
		json_skip_replace(handle->dblookup,(void*)dbserver_name,db);
		if(mysql_real_connect(db,NULL,NULL,NULL,NULL,0,NULL,CLIENT_MULTI_STATEMENTS) != NULL){
				return handle;
		}
	}
	return NULL;
}

db_handle_t * db_handle_init(const char *_user, const char *_pass, const char *_server, const char *_config, void *userarg,int multidb){
	db_handle_t *dbhandle= malloc(sizeof(db_handle_t));
	//should then be defined in the config if the user/pass is NULL
	dbhandle->user = _user ? strdup(_user) : NULL;
	dbhandle->pass = _pass ? strdup(_pass) : NULL;
	dbhandle->config = strdup(_config);
	dbhandle->db = NULL;
	dbhandle->dblookup = NULL;
	dbhandle->mpool = NULL;

	//chop up the server string
	int i = 0;
	char *server = strdup(_server);
	char *e = server;
	dbhandle->server_count = 1;
	while((e = strchr(e,':'))!=NULL) {dbhandle->server_count++; e++;}
	char *b =  e = server;
	dbhandle->server = malloc(sizeof(char*) * dbhandle->server_count);
	while((e = strchr(e,':'))!=NULL)  { 
		*e = '\0';
		dbhandle->server[i++] = strdup(b);
		e++;
		b = e;
	}
	dbhandle->server[i++] = strdup(b);
	unsigned int seed = *((unsigned int*)userarg);
	free(server);
	dbhandle->server_offset = seed  % dbhandle->server_count;
	if(multidb == 0) { 
		db_handle_reattach(dbhandle,"");
	} else {
		apr_pool_create(&(dbhandle->mpool),NULL);	
		dbhandle->dblookup = json_skip_create(dbhandle->mpool,4,(json_skip_cmp_t)strcmp);
		for(i=0;i < dbhandle->server_count; i++) { 
			db_handle_reattach(dbhandle,dbhandle->server[i]);
		}
	}
	return dbhandle;
}

void db_handle_destroy(db_handle_t *dbhandle) {
	int i; 
	if(dbhandle->dblookup == NULL) { 
		mysql_close(dbhandle->db);
	} else { 
		for(i = 0; i < dbhandle->server_count; i++) { 
			mysql_close((MYSQL*)json_skip_get(dbhandle->dblookup,dbhandle->server[i]));
		}	
		apr_pool_destroy(dbhandle->mpool);
	}

	mysql_thread_end();
	for(i = 0; i < dbhandle->server_count; i++) { 
		free(dbhandle->server[i]);	
	}
	free(dbhandle->server);
	free(dbhandle->config);
	free(dbhandle->user);
	free(dbhandle->pass);
	free(dbhandle);
}

json_value * dbresult2json(MYSQL_RES * myresult,apr_pool_t *mpool) { 
		json_value *result = json_object_create(mpool);
		unsigned int num_fields;
		unsigned int i;
		MYSQL_FIELD *fields;
		num_fields = mysql_num_fields(myresult);
		fields = mysql_fetch_fields(myresult);
		json_value *header = json_array_create(mpool,num_fields);
		json_object_add(result,"HEADER",header);
		json_value *coltypes = json_array_create(mpool,num_fields);
		json_object_add(result,"TYPES",coltypes);
		for(i = 0; i < num_fields; i++) {
			json_array_append(header,json_string_create(mpool,fields[i].name));
			switch(fields[i].type) {
					case MYSQL_TYPE_TINY: 
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_TINY")); break;
					case MYSQL_TYPE_SHORT:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_SHORT")); break;
					case MYSQL_TYPE_LONG: 
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_LONG")); break;
					case MYSQL_TYPE_INT24: 
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_INT24")); break;
					case MYSQL_TYPE_DECIMAL:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_DECIMAL")); break;
					case MYSQL_TYPE_NEWDECIMAL:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_NEWDECIMAL")); break;
					case MYSQL_TYPE_DOUBLE:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_DOUBLE")); break;
					case MYSQL_TYPE_FLOAT:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_FLOAT")); break;
					case MYSQL_TYPE_LONGLONG:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_LONGLONG")); break;
					case MYSQL_TYPE_BIT:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_BIT")); break;
					case MYSQL_TYPE_TIMESTAMP:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_TIMESTAMP")); break;
					case MYSQL_TYPE_DATE:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_DATE")); break;
					case MYSQL_TYPE_TIME:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_TIME")); break;
					case MYSQL_TYPE_DATETIME:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_DATETIME")); break;
					case MYSQL_TYPE_YEAR:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_YEAR")); break;
					case MYSQL_TYPE_STRING:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_STRING")); break;
					case MYSQL_TYPE_VAR_STRING:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_VAR_STRING")); break;
					case MYSQL_TYPE_NEWDATE:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_NEWDATE")); break;
					case MYSQL_TYPE_VARCHAR:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_VARCHAR")); break;
					case MYSQL_TYPE_BLOB:
						//DON'T ASK - it is not 63 it is TEXT 
						//http://www.mysql.org/doc/refman/5.1/en/c-api-datatypes.html
						if (fields[i].charsetnr == 63) {  
/**BINARY - NEEDS TO B64 **/
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_TEXT"));
						} else {
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_BLOB")); 
						}
						break;
					case MYSQL_TYPE_TINY_BLOB:
						if (fields[i].charsetnr == 63) {  
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_TEXT")); 
						} else {
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_TINY_BLOB")); 
						}
						break;
					case MYSQL_TYPE_MEDIUM_BLOB:
						if (fields[i].charsetnr == 63) {  
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_TEXT")); 
						} else {
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_MEDIUM_BLOB"));
						}
						break;
					case MYSQL_TYPE_LONG_BLOB:
						if (fields[i].charsetnr == 63) {  
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_TEXT")); 
						} else {
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_LONG_BLOB")); 
						}
						break;
					case MYSQL_TYPE_SET:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_SET")); break;
					case MYSQL_TYPE_ENUM:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_ENUM")); break;
					case MYSQL_TYPE_GEOMETRY:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_GEOMETRY")); break;
					case MYSQL_TYPE_NULL:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_NULL")); break;
					default:
							json_array_append(coltypes,json_string_create(mpool,"MYSQL_TYPE_UNKNOWN")); break;
					}
		}
		json_value *rows  = json_array_create(mpool,1000);
		json_object_add(result,"ROWS",rows);
		MYSQL_ROW myrow; 
		while( (myrow = mysql_fetch_row(myresult)) !=NULL) { 
			json_value *orow = json_array_create(mpool,num_fields);
			json_array_append(rows,orow);
			//unsigned long *lengths = mysql_fetch_lengths(myresult);
			for(i = 0; i < num_fields; i++) {
				switch(fields[i].type) {
					case MYSQL_TYPE_TINY:
					case MYSQL_TYPE_SHORT:
					case MYSQL_TYPE_LONG:
					case MYSQL_TYPE_INT24:
						json_array_append(orow,myrow[i] ? 
															json_long_create(mpool,atol(myrow[i] )) 
															:json_null_create(mpool));
						break;
					case MYSQL_TYPE_DECIMAL:
					case MYSQL_TYPE_NEWDECIMAL:
					case MYSQL_TYPE_DOUBLE:
					case MYSQL_TYPE_FLOAT:
					case MYSQL_TYPE_LONGLONG:
						json_array_append(orow,myrow[i] ? 
											json_double_create(mpool,atof(myrow[i]))
											:json_null_create(mpool));
						break;
					case MYSQL_TYPE_BIT:
					case MYSQL_TYPE_TIMESTAMP:
					case MYSQL_TYPE_DATE:
					case MYSQL_TYPE_TIME:
					case MYSQL_TYPE_DATETIME:
					case MYSQL_TYPE_YEAR:
					case MYSQL_TYPE_STRING:
					case MYSQL_TYPE_VAR_STRING:
					case MYSQL_TYPE_NEWDATE:
					case MYSQL_TYPE_VARCHAR:
						json_array_append(orow,
								myrow[i] ? json_string_create(mpool,myrow[i]) :
													json_null_create(mpool) );
						break;
					case MYSQL_TYPE_BLOB:
					case MYSQL_TYPE_TINY_BLOB:
					case MYSQL_TYPE_MEDIUM_BLOB:
					case MYSQL_TYPE_LONG_BLOB:
						//DON'T ASK - it is not 63 it is TEXT 
						//http://www.mysql.org/doc/refman/5.1/en/c-api-datatypes.html
						if (fields[i].charsetnr == 63) {  
/**BINARY - NEEDS TO B64 **/
							json_array_append(orow,json_null_create(mpool));
						} else {
							json_array_append(orow,
									myrow[i] ? json_string_create(mpool,myrow[i]) :
														json_null_create(mpool) );
						}
						break;
					case MYSQL_TYPE_SET:
					case MYSQL_TYPE_ENUM:
					case MYSQL_TYPE_GEOMETRY:
					case MYSQL_TYPE_NULL:
							json_array_append(orow,json_null_create(mpool));
						break;
					}
			}
		}
	return result;
}

json_value * dbexecute(db_handle_t *dbhandle, json_value *injson, apr_pool_t *mpool) {
	json_value *out = json_object_create(mpool);
	json_value *sql = NULL;
	MYSQL *db = NULL; //dbhandle->db;
	const char *dbserver_name = NULL;//dbhandle->server[xdbhandle->server_offset];
	json_value *request_server = NULL;

	// CHECK FOR SERVER in the incoming JSON - and pick out flag - 
	if(injson->type == JSON_OBJECT && (request_server = (json_value*)json_skip_get(injson->value.object,"SERVER")) !=NULL && request_server->type == JSON_STRING) {
		if(dbhandle->dblookup == NULL)  { 
			json_object_add(out,"ERROR",json_string_create(mpool,"Provided a SERVER argument but dbslayer is not configure for named server access - perhaps you want the -m config option"));
			return out;
		}
		dbserver_name = request_server->value.string;
		db = (MYSQL*) json_skip_get(dbhandle->dblookup,(void*)dbserver_name);
		if(db == NULL) { 
			json_object_add(out,"ERROR",json_string_create(mpool,"Couldn't find the database handle for SERVER requested"));
			json_object_add(out,"SERVER",request_server);
			return out;
		}
		
	} else if(dbhandle->dblookup !=NULL) { 
		json_object_add(out,"ERROR",json_string_create(mpool,"dbslayer is configured to take a SERVER argument - perhaps you want the -s config option"));
		return out;
	} else { 
		db = dbhandle->db;
		dbserver_name = dbhandle->server[dbhandle->server_offset];
	}
	
	if(injson->type == JSON_OBJECT && (sql = (json_value*)json_skip_get(injson->value.object,"SQL")) !=NULL && sql->type == JSON_STRING) { 
		if(mysql_query(db,sql->value.string)) { 
/** NEED TO CHECK FOR BETTER ERROR TO MAKE SURE IT IS A CONNECTION ISSUE **/
			if(db_handle_reattach(dbhandle,dbserver_name) == NULL || mysql_query((db = dbhandle->dblookup == NULL ? dbhandle->db : json_skip_get(dbhandle->dblookup,(void*)dbserver_name)) ,sql->value.string)){

				dbserver_name = dbhandle->dblookup == NULL ? dbhandle->server[dbhandle->server_offset] : dbserver_name;
				json_object_add(out,"MYSQL_ERROR",json_string_create(mpool,mysql_error(db)));
				json_object_add(out,"MYSQL_ERRNO",json_long_create(mpool,mysql_errno(db)));
				json_object_add(out,"SERVER" , json_string_create(mpool,dbserver_name));

        /** issue a rollback if caller asked for it **/
        if ((injson->type == JSON_OBJECT)
              && ((sql = (json_value*)json_skip_get(injson->value.object, "ROLLBACK_ON_ERROR" )) != NULL)
              && (sql->type == JSON_BOOLEAN)
              && sql->value.boolean) {
          
          json_object_add(out, "ROLLBACK_ON_ERROR", json_boolean_create(mpool, 1));
          json_object_add(out, "ROLLBACK_ON_ERROR_SUCCESS", json_boolean_create(mpool, !mysql_rollback(db)));
 
	      }
          
				return out;
			}
		}
		json_value *all_result = NULL;
		json_value *sql_result = NULL;
		int status = 0;
		do {
			MYSQL_RES *myresult = mysql_store_result(db);
			if(myresult == NULL  ) {  
				/** ERROR OCCURED ***/
				if(mysql_errno(db)) {
					json_object_add(out,"SUCCESS",json_boolean_create(mpool,0));
					json_object_add(out,"MYSQL_ERROR",json_string_create(mpool,mysql_error(db)));
					json_object_add(out,"MYSQL_ERRNO",json_long_create(mpool,mysql_errno(db)));
					json_object_add(out,"SERVER" , json_string_create(mpool,dbserver_name));

          /** issue a rollback if caller asked for it **/
          if ((injson->type == JSON_OBJECT)
                && ((sql = (json_value*)json_skip_get(injson->value.object, "ROLLBACK_ON_ERROR")) != NULL)
                && (sql->type == JSON_BOOLEAN)
                && sql->value.boolean) {

            json_object_add(out, "ROLLBACK_ON_ERROR", json_boolean_create(mpool, 1));
            json_object_add(out, "ROLLBACK_ON_ERROR_SUCCESS", json_boolean_create(mpool, !mysql_rollback(db)));

  	      }

				} else {
					/** SUCCESS NO RESULT RETURNED ie UPDATE | DELETE | INSERT ***/
					;
					if(sql_result && all_result ) { 
						sql_result = json_object_create(mpool);
						json_object_add(sql_result,"SUCCESS",json_boolean_create(mpool,1));
						json_object_add(sql_result,"AFFECTED_ROWS",json_long_create(mpool,(long)mysql_affected_rows(db)));
						json_object_add(sql_result,"INSERT_ID",json_long_create(mpool,(long)mysql_insert_id(db)));
						json_array_append(all_result,sql_result);
					} else if (sql_result && !all_result)  { 
						all_result = json_array_create(mpool,5);
						json_array_append(all_result,sql_result);
						sql_result = json_object_create(mpool);
						json_object_add(sql_result,"SUCCESS",json_boolean_create(mpool,1));
						json_object_add(sql_result,"AFFECTED_ROWS",json_long_create(mpool,(long)mysql_affected_rows(db)));
						json_object_add(sql_result,"INSERT_ID",json_long_create(mpool,(long)mysql_insert_id(db)));
						json_array_append(all_result,sql_result);
						json_object_add(out,"RESULT",all_result);
					} else { 
						sql_result = json_object_create(mpool);
						json_object_add(sql_result,"SUCCESS",json_boolean_create(mpool,1));
						json_object_add(sql_result,"AFFECTED_ROWS",json_long_create(mpool,(long)mysql_affected_rows(db)));
						json_object_add(sql_result,"INSERT_ID",json_long_create(mpool,(long)mysql_insert_id(db)));
					}
				}
			} else { 
				if(sql_result && all_result ) { 
					json_array_append(all_result,dbresult2json(myresult,mpool));
				} else if (sql_result && !all_result)  { 
					all_result = json_array_create(mpool,5);
					json_array_append(all_result,sql_result);
					json_array_append(all_result,dbresult2json(myresult,mpool));
					json_object_add(out,"RESULT",all_result);
				} else { 
					sql_result = dbresult2json(myresult,mpool);
				}
				mysql_free_result(myresult);
			}
			if ((status = mysql_next_result(db)) > 0) printf("Could not execute statement\n");
		}while(status ==0);

		if(sql_result && !all_result) { 
			json_object_add(out,"RESULT",sql_result);
		}
		json_object_add(out,"SERVER" , json_string_create(mpool,dbserver_name));
	} 
	if(injson->type == JSON_OBJECT && (sql = (json_value*)json_skip_get(injson->value.object,"STAT")) !=NULL && sql->type == JSON_BOOLEAN && sql->value.boolean) { 
		json_object_add(out,"STAT",json_string_create(mpool,mysql_stat(db)));
	}
	if(injson->type == JSON_OBJECT && (sql = (json_value*)json_skip_get(injson->value.object,"CLIENT_INFO")) !=NULL && sql->type == JSON_BOOLEAN && sql->value.boolean) { 
		json_object_add(out,"CLIENT_INFO",json_string_create(mpool,mysql_get_client_info()));
	}
	if(injson->type == JSON_OBJECT && (sql = (json_value*)json_skip_get(injson->value.object,"HOST_INFO")) !=NULL && sql->type == JSON_BOOLEAN && sql->value.boolean) { 
		json_object_add(out,"HOST_INFO",json_string_create(mpool,mysql_get_host_info(db)));
	}
	if(injson->type == JSON_OBJECT && (sql = (json_value*)json_skip_get(injson->value.object,"SERVER_VERSION")) !=NULL && sql->type == JSON_BOOLEAN && sql->value.boolean) { 
		json_object_add(out,"SERVER_VERSION",json_long_create(mpool,mysql_get_server_version(db)));
	}
	if(injson->type == JSON_OBJECT && (sql = (json_value*)json_skip_get(injson->value.object,"CLIENT_VERSION")) !=NULL && sql->type == JSON_BOOLEAN && sql->value.boolean) { 
		json_object_add(out,"CLIENT_VERSION",json_long_create(mpool,mysql_get_client_version()));
	}
	if(injson->type == JSON_OBJECT && (sql = (json_value*)json_skip_get(injson->value.object,"SLAYER_DEBUG_RETURN_INPUT")) !=NULL && sql->type == JSON_BOOLEAN && sql->value.boolean) { 
		json_object_add(out,"SLAYER_DEBUG_RETURN_INPUT",injson);
	}
	if(injson->type == JSON_OBJECT && (sql = (json_value*)json_skip_get(injson->value.object,"SERVER_NAME")) !=NULL && sql->type == JSON_BOOLEAN && sql->value.boolean) { 
		json_object_add(out,"SERVER_NAME",json_string_create(mpool,dbserver_name));
	}
	if(injson->type == JSON_OBJECT && (sql = (json_value*)json_skip_get(injson->value.object,"SLAYER_HELP")) !=NULL && sql->type == JSON_BOOLEAN && sql->value.boolean) { 
		json_value *commands = json_object_create(mpool);
		json_object_add(out,"SLAYER_HELP",commands);
		json_object_add(commands,"SQL",json_string_create(mpool,"in(string)[sql statement to execute] : out()eturns results in RESULTS node"));
		json_object_add(commands,"STAT",json_string_create(mpool,"in(boolean) : out(string) result of mysql_stat()"));
		json_object_add(commands,"CLIENT_INFO",json_string_create(mpool,"in(boolean) : out(string) result of mysql_get_client_info()"));
		json_object_add(commands,"HOST_INFO",json_string_create(mpool,"in(boolean) : out(string) result of mysql_get_host_info()"));
		json_object_add(commands,"SERVER_VERSION",json_string_create(mpool,"in(boolean) : out(long) result of mysql_server_version()"));
		json_object_add(commands,"CLIENT_VERSION",json_string_create(mpool,"in(boolean) : out(long) result of mysql_client_version()"));
		json_object_add(commands,"SLAYER_HELP",json_string_create(mpool,"in(boolean) : true ? returns this result"));
		json_object_add(commands,"SLAYER_DEBUG_RETURN_INPUT",json_string_create(mpool,"in(boolean) : true ? returns this result"));
		json_object_add(commands,"SERVER_NAME",json_string_create(mpool,"in(boolean) : true ? returns stanza from the -s option that was used"));
	}
	if(out->type == JSON_OBJECT && out->value.object->node_count == 0) { 
		json_object_add(out,"ERROR",json_string_create(mpool,"TRY {\"SLAYER_HELP\":true } "));
	}
	return out;
}
