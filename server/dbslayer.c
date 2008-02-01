#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <assert.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <apr_thread_proc.h>
#include <apr_queue.h>
#include <apr_poll.h>
#include <apr_atomic.h>
#include <apr_time.h>
#include <apr_file_info.h>
#include <apr_md5.h> 
#ifdef HAVE_LUA5_1_LUA_H
#include <lua5.1/lua.h>
#include <lua5.1/lualib.h>
#include <lua5.1/lauxlib.h>
#define HAVE_LUA 1
#else
#ifdef HAVE_LUA_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#define HAVE_LUA 1
#endif
#endif
#include "dbaccess.h"
#include "simplejson.h"
#include "slayer_util.h"
#include "dbslayer.h"
#include "dbslayer_logging.h"
#include "dbslayer_stats.h"
#include "json_parser_lua.h"
#ifdef HAVE_APR_MEMCACHE_H
#include <apr_memcache-0/apr_memcache.h>
#define APR_MEMCACHE_DEFAULT_TTL 300000
#endif
#include "dbslayer_data_types.h"
/* $Id: dbslayer.c,v 1.26 2007/07/10 17:55:16 derek Exp $ */
#define RFC_822_DATE_FMT "%a, %d %b %Y %H:%M:%S %z"
#define SLAYER_SERVER "Server: dbslayer/server beta-10\r\n"

//Content-type= text/plain; \r\n

int handle_response(thread_shared_data_t *td, queue_data_t *qd, char *equery,
		const char *http_request, const char *header_response,
		int response_code, const char *message) {
	char dstring[APR_RFC822_DATE_LEN];
	apr_rfc822_date(dstring, apr_time_now());
	char *response =
			apr_pstrcat(qd->mpool, "HTTP/1.0 ", header_response, "\r\n",
					"Date: ", dstring, "\r\n", 
					SLAYER_SERVER, "Connection: Close\r\n",
					"Content-type: text/plain; charset=utf-8\r\n",
					"Content-Length: ", apr_itoa(qd->mpool, strlen(message)),
					"\r\n", "\r\n", message, NULL);
	int rlen = strlen(response);
	apr_size_t r =0;
	apr_size_t sent = 0;
	apr_status_t status;
	do {
		r = rlen - sent;
		status = apr_socket_send(qd->conn, response + sent, &r);
		sent += r;
	} while (status != APR_TIMEUP && r!=0 && sent < rlen);
	if ((r == 0 && sent < rlen) || status == APR_TIMEUP) {
		dbslayer_log_err_message(td->elmanager, qd->mpool, qd->conn,
				http_request, "Connection timed out");
	}
	dbslayer_log_request(td->lmanager, qd->mpool, qd->conn, http_request,
			response_code, sent, apr_time_now() - qd->begin_request);
	return 0;
}

typedef struct _slayer_mtypes_t {
	char *suffix;
	char *mtype;
} slayer_mtypes_t;

slayer_mtypes_t mtypes[] = { { ".html", "text/html" }, { ".png", "image/png" },
		{ ".gif", "image/gif" }, { ".jpg", "image/jpeg" },
		{ ".xml", "text/xml" }, { ".xsl", "text/xml" }, { NULL, NULL } };

int handle_other(thread_shared_data_t *td, queue_data_t *qd, char *equery,
		const char * http_request) {
	if (td->basedir && equery && strstr(equery, "..") == NULL) {
		apr_status_t status;
		apr_file_t *outfile;
		apr_finfo_t outfile_stat;
		equery = strcmp(equery, "/") == 0 ? "/index.html" : equery;
		char *out = apr_pstrcat(qd->mpool, td->basedir, equery, NULL);
		status = apr_stat(&outfile_stat, out, APR_FINFO_SIZE, qd->mpool);
		status = apr_file_open(&outfile, out, APR_READ, APR_OS_DEFAULT,
				qd->mpool);
		if (status == APR_SUCCESS) {
			apr_off_t off = 0;
			apr_size_t sent = 0;
			apr_size_t outlen = 0;
			char dstring[APR_RFC822_DATE_LEN];
			apr_rfc822_date(dstring, apr_time_now());
			char *end = equery + strlen(equery);
			while (*end != '.' && end != equery)
				end--;
			char *content_type = "text/plain";
			if ( *end == '.') {
				int i = 0;
				for (i = 0; mtypes[i].suffix; i++) {
					if (strcasecmp(end, mtypes[i].suffix) == 0) {
						content_type = mtypes[i].mtype;
						break;
					}
				}
			}
			char *response =
					apr_pstrcat(qd->mpool, "HTTP/1.0 200 OK\r\n", "Date: ",
							dstring, "\r\n", 
							SLAYER_SERVER, "Connection: Close\r\n",
							"Content-type: ", content_type, "\r\n",
							"Content-Length: ", apr_itoa(qd->mpool,
									outfile_stat.size), "\r\n", "\r\n", NULL);
			int rlen = strlen(response);
			apr_size_t r =0;
			sent = 0;
			apr_status_t status;
			//SEND HEADER
			do {
				r = rlen - sent;
				status = apr_socket_send(qd->conn, response + sent, &r);
				sent += r;
			} while (status != APR_TIMEUP && r!=0 && sent < rlen);
			if ((r == 0 && sent < rlen) || status == APR_TIMEUP) {
				dbslayer_log_err_message(td->elmanager, qd->mpool, qd->conn,
						http_request, "Connection timed out");
				return 0;
			}
#ifdef DARWIN
			dbslayer_log_err_message(td->elmanager,qd->mpool,qd->conn,http_request,"Sendfile not supported on osx/darwin, dang.");
			//SEND FILE
			sent = 0;
#else
			do {
			    outlen = outfile_stat.size - sent;
			    off = sent;
			    apr_socket_sendfile(qd->conn, outfile, NULL, &off, &outlen, 0);
			    sent += outlen;



			} while (status != APR_TIMEUP && outlen!=0 && sent
					< outfile_stat.size);
#endif
			if ((outlen == 0 && sent < outfile_stat.size) || status
			    == APR_TIMEUP) {
			    dbslayer_log_err_message(td->elmanager, qd->mpool, qd->conn,
						     http_request, "Connection timed out");

			    return 0;
			}

			apr_file_close(outfile);

			dbslayer_log_request(td->lmanager, qd->mpool, qd->conn,
					http_request, 200, sent, apr_time_now() - qd->begin_request);
			return 0;
		}
	}
	return handle_response(td, qd, equery, http_request, "404 Not Found", 404,
			"Not Found");
}

#ifdef HAVE_LUA
char * execute_lua(lua_State *L, char *filter, char *input, apr_pool_t *mpool) {
	int status;
	char *lua_output=NULL;
	char *output=NULL;
	lua_pushstring(L, input);
	lua_setglobal(L, SLAYER_LUA_INPUT_VAR);
	status = luaL_dofile(L, filter);
	if (status) {
		printf("ERROR: lua - %s\n", (char *)lua_tostring(L, -1));
		lua_pop(L, 1);
		output = apr_pstrdup(mpool, input);
		return output;
	}
	lua_output = (char *)lua_tostring(L, -1);
	lua_pop(L, 1);
	output = apr_pstrdup(mpool, lua_output);
	return output;
}
#endif
#ifdef HAVE_APR_MEMCACHE_H

apr_status_t cache_key_to_hash(thread_shared_data_t *td, const char *key, char *key_hash, size_t key_hash_len) {
	unsigned char digest[APR_MD5_DIGESTSIZE];
	apr_status_t ret;
	memset(digest, 0, APR_MD5_DIGESTSIZE); 
	ret = apr_md5(digest, key, strlen(key));
	if ( ret == APR_SUCCESS ) {
		snprintf(key_hash, key_hash_len, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	            digest[0], digest[1], digest[2], digest[3],
	            digest[4], digest[5], digest[6], digest[7],
	            digest[8], digest[9], digest[10], digest[11],
	            digest[12], digest[13], digest[14], digest[15]);
	}
	return ret;
}
apr_status_t cache_set(thread_shared_data_t *td, queue_data_t *qd,
		json_value *key, json_value *value,
		apr_short_interval_time_t cache_until) {
	char key_hash[1024];
	char *serialized_value;
	cache_key_to_hash(td, key->value.string, key_hash, 1024);
	serialized_value = json_serialize(qd->mpool, value);
	return apr_memcache_set(td->memcache, key_hash, serialized_value,
			(strlen(serialized_value) + 1), cache_until, 0);
}

#endif
apr_status_t cache_result(thread_shared_data_t *td, queue_data_t *qd,
		json_value *query, json_value *result, const char *http_request) {
	apr_status_t rv= APR_ENOTIMPL;	
#ifdef HAVE_APR_MEMCACHE_H
	long cache_ttl;
	json_value *sql;
	apr_short_interval_time_t cache_until;
	char tm[128];
	struct tm *l_tm;
	time_t t;
	if (td->memcache != NULL) {
		if (json_get_sql(query, &sql) == APR_SUCCESS) {
			if (json_allows_caching(query) ) {
				if (json_get_cache_ttl(query, &cache_ttl) != APR_SUCCESS) {
					cache_ttl = APR_MEMCACHE_DEFAULT_TTL;
				}
				t = time(NULL);
				l_tm = localtime(&t);
				strftime(tm, sizeof(tm), RFC_822_DATE_FMT, l_tm);
				json_add_object(result, "CACHED", json_create_string(qd->mpool,
										tm));
				cache_until = apr_time_make(cache_ttl,0);
				rv = cache_set(td, qd, sql, result, cache_until);
				json_add_object(result, "CACHE_WRITE", json_create_long(qd->mpool,
										1));				
				if (rv != APR_SUCCESS) {
					dbslayer_log_err_message(td->elmanager, qd->mpool,
							qd->conn, http_request,
							"ERROR: Memcache set failed.");
				}
			}
		}
	}
#endif	
	return rv;
}

char * query_db(thread_shared_data_t *td, queue_data_t *qd,
		db_handle_t *dbhandle, json_value *stmt, const char *http_request) {

	char *out_json =
			"{\"ERROR\" : \"we got problems -  couldn't parse your incoming json\"}";
	if (dbhandle && stmt) {
		json_value *result = db_execute(dbhandle, stmt, qd->mpool);
		json_value *errors = apr_hash_get(result->value.object, "MYSQL_ERROR", 
		APR_HASH_KEY_STRING);
		if (errors) {
			dbslayer_log_err_message(td->elmanager, qd->mpool, qd->conn,
					http_request, apr_pstrcat(qd->mpool, "ERROR: ",
							errors->value.string, NULL));
		} else {
			cache_result(td, qd, stmt, result, http_request);
		}
		out_json = json_serialize(qd->mpool, result);

	} else {
		dbslayer_log_err_message(td->elmanager, qd->mpool, qd->conn,
				http_request, "ERROR: Couldn't parse incoming JSON");
	}
	return out_json;
}

#ifdef HAVE_APR_MEMCACHE_H


apr_status_t cache_get(thread_shared_data_t *td, queue_data_t *qd,
		json_value *key, char **out_json, apr_size_t *len) {
	char key_hash[64];
	apr_status_t retval;
	retval = cache_key_to_hash(td, key->value.string, key_hash, 64);
	if (retval == APR_SUCCESS) {
	  retval = apr_memcache_getp(td->memcache, qd->mpool, key_hash,
			out_json, len, NULL);
	}
	return retval;
}
#endif 
char * query_cache(thread_shared_data_t *td, queue_data_t *qd,
		db_handle_t *dbhandle, char *in_json, const char *http_request) {
	char *out_json;
#ifdef HAVE_APR_MEMCACHE_H
	json_value *sql=NULL;
	apr_status_t rv;
	apr_size_t len;
#endif
	json_value *stmt = decode_json(in_json, qd->mpool);
#ifdef HAVE_APR_MEMCACHE_H
	if (td->memcache != NULL) {
		if ( (json_get_sql(stmt, &sql) == APR_SUCCESS)
				&& json_allows_caching(stmt) ) {
			rv = cache_get(td, qd, sql, &out_json, &len);
			if (rv == APR_SUCCESS) {
				return out_json;
			}
		}
	}
#endif
	out_json = query_db(td, qd, dbhandle, stmt, http_request);

	return out_json;
}

char *handle_lua_filter(thread_shared_data_t *td, thread_uniq_data_t *tu,
		queue_data_t *qd, db_handle_t *dbhandle, char *equery,
		const char *http_request) {
	char *output=NULL;

	char *input = urldecode(qd->mpool, equery);
#ifdef HAVE_LUA
	if (td->input_filter_lua == NULL) {
		output = apr_pstrdup(qd->mpool, input);
	} else {
		output = execute_lua(tu->lua_state, td->input_filter_lua, input,
				qd->mpool);
	}

	input = query_cache(td, qd, dbhandle, output, http_request);
	if (td->output_filter_lua == NULL) {
		output = apr_pstrdup(qd->mpool, input);
	} else {
		output = execute_lua(tu->lua_state, td->output_filter_lua, input,
				qd->mpool);
	}

#else
	output = query_cache(td, qd, dbhandle, input, http_request);
#endif
	return output;
}

int handle_query(thread_shared_data_t *td, thread_uniq_data_t *tu,
		queue_data_t *qd, db_handle_t *dbhandle, char *equery,
		const char *http_request) {

	char *output=NULL;
	output = handle_lua_filter(td, tu, qd, dbhandle, equery, http_request);

	handle_response(td, qd, equery, http_request, "200 OK", 200, output);
	return 0;
}

int uri_match(apr_uri_t *uri_info, char *path, apr_pool_t *mpool) {
	char *path_cpy= NULL;
	char *slashed_path= NULL;
	int retval = 0;
	path_cpy = apr_pstrdup(mpool, path);
	slashed_path = apr_pstrcat(mpool, slashed_path, "/");
	if (strcmp(uri_info->path, path) == 0 || strcmp(uri_info->path,
			slashed_path) == 0) {
		retval = 1;
	}

	return retval;
}

void handle_mapping(thread_shared_data_t *td, thread_uniq_data_t *tu,
		apr_pool_t *mpool) {
#ifdef HAVE_LUA
	if (td->mapper_lua != NULL) {
		if (td->serialized_mapping == NULL) {
			if (luaL_dofile(tu->lua_state, td->mapper_lua)) {
				printf("ERROR: lua - %s\n", (char *)lua_tostring(tu->lua_state,
						-1));
				lua_pop(tu->lua_state, 1);
			} else {
				char *lua_output=NULL;
				lua_output = (char *)lua_tostring(tu->lua_state, -1);
				lua_pop(tu->lua_state, 1);
				td->serialized_mapping = apr_pstrdup(mpool, lua_output);
			}
		}
	}
#endif	
}

apr_status_t handle_connection(thread_shared_data_t *td,
		thread_uniq_data_t *tu, queue_data_t *qd, db_handle_t *dbhandle) {
	char buf[1024];
	apr_size_t bufsize;
	char *request;
	char *http_line;
	char *request_tokenizer;
	apr_status_t status;
	apr_socket_t *conn;
	apr_pool_t *mpool;

	conn = qd->conn;
	mpool = qd->mpool;

	memset(buf, 0, sizeof(buf));
	bufsize = sizeof(buf)-1;
	status = apr_socket_recv(conn, buf, &bufsize);
	if (bufsize == 0)
		goto terminate;
	// means it timedout
	request = apr_pstrdup(mpool, buf);
	while (status == APR_SUCCESS && strstr(request, "\r\n\r\n") == NULL
			&& strstr(request, "\n\n") == NULL && strstr(request, "\r\r")
			== NULL) {
		memset(buf, 0, sizeof(buf));
		bufsize = sizeof(buf)-1;
		status = apr_socket_recv(conn, buf, &bufsize);
		if (bufsize == 0 || bufsize >strlen(buf) )
			goto terminate;
		// means it timedout
		request = apr_pstrcat(mpool, request, buf, NULL);
	}
	if (status != APR_SUCCESS)
		goto terminate;
	http_line = apr_strtok(request, "\n\r", &request_tokenizer);
	if (http_line) {
		char *line_tokenizer;
		char *meat;
		char *token;
		apr_uri_t uri_info;
		char *http_request = apr_pstrdup(mpool, http_line);

		token = apr_strtok(http_line, " \t", &line_tokenizer);
		if (token == NULL || strcmp(token, "GET") != 0)
			goto terminate;
		meat = apr_strtok(NULL," \t",&line_tokenizer);
		if (meat == NULL)
			goto terminate;
		token = apr_strtok(NULL," \t",&line_tokenizer);
		if (token == NULL || strncmp(token, "HTTP/1", 6) != 0)
			goto terminate;
		apr_uri_parse(mpool, meat, &uri_info);
		if (uri_match(&uri_info, SLAYER_QUERY_PATH, mpool) && uri_info.query
				!= NULL) {
			handle_query(td, tu, qd, dbhandle, uri_info.query, http_request);
		} else if (uri_match(&uri_info, SLAYER_MAPPING_PATH, mpool)) {
			handle_mapping(td, tu, mpool);
			handle_response(td, qd, uri_info.path, http_request, "200 OK", 200,
					td->serialized_mapping);
		} else if (uri_match(&uri_info, SLAYER_SCHEMA_PATH, mpool)) {
			handle_response(td, qd, uri_info.path, http_request, "200 OK", 200,
					td->serialized_schema);
		} else if (uri_match(&uri_info, SLAYER_SHUTDOWN_PATH, mpool) ) {
			apr_sockaddr_t *local_addr;
			apr_sockaddr_t *remote_addr;
			char *local_ip;
			char *remote_ip;
			apr_socket_addr_get(&remote_addr, APR_REMOTE, conn);
			apr_sockaddr_ip_get(&remote_ip, remote_addr);
			apr_socket_addr_get(&local_addr, APR_LOCAL, conn);
			apr_sockaddr_ip_get(&local_ip, local_addr);
			if (strcmp(local_ip, remote_ip) == 0) {
				handle_response(td, qd, uri_info.path, http_request, "200 OK",
						200, "Shutting down");
				apr_socket_close(conn);
				return APR_EOF;
			} else {
				handle_response(td, qd, uri_info.path, http_request,
						"403 Forbidden", 403, "You are not authorized!");
			}
		} else if (uri_match(&uri_info, SLAYER_STATS_PATH, mpool) ) {
			char *stats = dbslayer_stats_tojson(td->stats, qd->mpool);
			handle_response(td, qd, uri_info.path, http_request, "200 OK", 200,
					stats);
		} else if (uri_match(&uri_info, SLAYER_STATS_LOG_PATH, mpool) ) {
			char *queries = dbslayer_log_get_entries(td->lmanager, qd->mpool);
			handle_response(td, qd, uri_info.path, http_request, "200 OK", 200,
					queries);
		} else if (uri_match(&uri_info, SLAYER_STATS_ERROR_PATH, mpool) ) {
			char *errors= dbslayer_log_get_entries(td->elmanager, qd->mpool);
			handle_response(td, qd, uri_info.path, http_request, "200 OK", 200,
					errors);
		} else if (uri_match(&uri_info, SLAYER_STATS_ARGS_PATH, mpool) ) {
			handle_response(td, qd, uri_info.path, http_request, "200 OK", 200,
					td->startup_args);
		} else {
			handle_other(td, qd, uri_info.path, http_request);
		}
		dbslayer_stat_update(td->stats);
	}
	terminate: status = apr_socket_close(conn);
	return APR_SUCCESS;
}

void* run_query_thread(apr_thread_t *mythread, void * x) {
	void *fetch= NULL;
	apr_pool_t *mpool;
	apr_status_t status= apr_pool_create(&mpool,NULL);
	thread_wrapper_data_t *td = (thread_wrapper_data_t*) x;
	db_handle_t *dbhandle = db_handle_init(td->shared->user, td->shared->pass,
			td->shared->server, td->shared->config, &(td->uniq->thread_number));
	/* need to set this here, needs a dbhandle */
	if (td->shared->serialized_schema == NULL) {
		td->shared->serialized_schema = json_serialize(mpool, db_schema(
				dbhandle, mpool));
	}
#ifdef HAVE_LUA	    
	lua_pushstring(td->uniq->lua_state, td->shared->serialized_schema);
	lua_setglobal(td->uniq->lua_state, SLAYER_LUA_SCHEMA_VAR);
	/* lua_pushstring(td->uniq->lua_state, dbhandle->server[dbhandle->server_offset]);
	 lua_setglobal(td->uniq->lua_state, SLAYER_LUA_SERVER_VAR); */
#endif

	do {
		fetch = NULL;;
		while ( (status = apr_queue_pop(td->shared->in_queue, &fetch))
				== APR_EINTR)
			;
		if (status == APR_SUCCESS) {
			queue_data_t *qd = (queue_data_t*) fetch;

			status = handle_connection(td->shared, td->uniq, qd, dbhandle);
			if (status == APR_SUCCESS) {
				while ((status == apr_queue_push(td->shared->out_queue, qd))
						== APR_EINTR)
					;
			} else {
				apr_queue_term(td->shared->out_queue);
				apr_queue_term(td->shared->in_queue);
				apr_atomic_set32(&(td->shared->shutdown), 1);
			}
		}
	} while (status == APR_SUCCESS);
	if (dbhandle)
		db_handle_destroy(dbhandle);
	apr_pool_destroy(mpool);
	return NULL;
}

apr_status_t serve_requests(apr_socket_t *conn, char *ebuf,
		apr_array_header_t *threads, int thread_count,
		apr_threadattr_t *thread_attr, thread_shared_data_t *td_shared,
		char *preload_lua, apr_queue_t *out_queue, apr_queue_t *in_queue,
		apr_pollset_t *pollset, char *reldir, int socket_timeout,
		apr_pool_t *mpool) {
	const apr_pollfd_t *ret_pfd;
	int pools_created = 0;
	int serviced = 0;
	int i;
	int events;
	apr_status_t status;
	while (1) {
		queue_data_t *qd= NULL;
		void *fetch= NULL;
		while ((status = apr_queue_trypop(in_queue, &fetch)) == APR_EINTR)
			;
		qd = (queue_data_t*)fetch;
		if (status == APR_SUCCESS) {
			apr_pool_clear(qd->mpool);
		} else if (status == APR_EAGAIN) {
			qd = apr_pcalloc(mpool,sizeof(queue_data_t));
			apr_pool_create(&(qd->mpool),NULL);
			pools_created +=1;
		} else if (status == APR_EOF) {
			goto shutdown;
		}
		do {
			status = apr_pollset_poll(pollset, 1000*1000 /* .5 sec */, &events,
					&ret_pfd);
			if (apr_atomic_read32(&(*td_shared).shutdown) )
				goto shutdown;
		} while (status == APR_TIMEUP);
		if (status == APR_SUCCESS) {
			status = apr_socket_accept(&(qd->conn), conn, qd->mpool);
			status = apr_socket_timeout_set(qd->conn, socket_timeout);
			qd->begin_request = apr_time_now();
			while ((status == apr_queue_push(out_queue, qd)) == APR_EINTR)
				;
			if (status == APR_EOF)
				goto shutdown;
			serviced++;
		} else {
			//dlh
			fprintf(stderr,"ERROR in apr_pollset_poll - %s\n",apr_strerror(status,ebuf,sizeof(ebuf)));
			dbslayer_log_message(td_shared->elmanager,
					"ERROR in apr_pollset_poll - ");
			dbslayer_log_message(td_shared->elmanager, apr_strerror(status,
					ebuf, sizeof(ebuf)));
			dbslayer_log_message(td_shared->elmanager, "\n");
		}
	}

	shutdown: for (i = 0; i < thread_count; i++) {
		apr_thread_t *thread = *((apr_thread_t**)(threads->elts
				+ (threads->elt_size * i)));
		apr_thread_join(&status, thread);
	}

	return APR_SUCCESS;

}
apr_status_t create_threads(apr_array_header_t *threads, int thread_count,
		apr_threadattr_t *thread_attr, thread_shared_data_t *td_shared,
		char *preload_lua, apr_pool_t *mpool) {
	int i;
	for (i = 0; i < thread_count; i++) {
#ifdef HAVE_LUA		
		lua_State *L;
#endif		
		apr_thread_t *thread;
		thread_wrapper_data_t *td_wrapper= apr_pcalloc(mpool,
				sizeof(thread_wrapper_data_t));
		td_wrapper->shared = td_shared;
		td_wrapper->uniq = apr_pcalloc(mpool,
				sizeof(thread_uniq_data_t));
		td_wrapper->uniq->thread_number = i;
#ifdef HAVE_LUA
		L = luaL_newstate();
		td_wrapper->uniq->lua_state = L;
		luaL_openlibs(L);
		if (luaL_dostring(L, json_parser_lua())) {
			printf("ERROR: lua - %s\n", (char *)lua_tostring(L, -1));
			lua_pop(td_wrapper->uniq->lua_state, 1);
		}
		if (preload_lua != NULL) {
			if (luaL_dofile(L, preload_lua)) {
				printf("ERROR: lua - %s\n", (char *)lua_tostring(L, -1));
				lua_pop(td_wrapper->uniq->lua_state, 1);
			}
		}
#endif
		apr_thread_create(&thread, thread_attr, run_query_thread, td_wrapper,
				mpool);
		*((apr_thread_t**)(apr_array_push(threads))) = thread;
	}
	return APR_SUCCESS;
}

int main(int argc, char **argv) {
	apr_pool_t *mpool;
	apr_status_t status;
	apr_socket_t *conn;
	apr_sockaddr_t *addr;
	apr_pollset_t *pollset;
	apr_pollfd_t pfd;
	apr_threadattr_t *thread_attr;
	apr_array_header_t *threads;
	int i, thread_count =1;
	apr_queue_t *out_queue, *in_queue;
	thread_shared_data_t td_shared;
	char *hostname= NULL;
	char *user= NULL;
	char *pass= NULL;
	char *server= NULL;
	char *config= NULL;
	char *debug= NULL;
	char *basedir= NULL;
	char *logfile= NULL;
	char *elogfile= NULL;
#ifdef HAVE_APR_MEMCACHE_H	
	char *memcache_host= NULL;
	apr_port_t memcache_port = -1;
	apr_status_t rv;
#endif
	int port = 9090;
	int option;
	int socket_timeout = 10 * 1000* 1000;
	int nslice = 60 * 24;
	int tslice = 60;
	char ebuf[1024];
	char *reldir= NULL;
	char *output_filter= NULL;
	char *input_filter= NULL;
	char *mapper_lua= NULL;
	char *preload_lua= NULL;

	while ((option = getopt(argc, argv,
			"t:h:p:u:x:s:c:d:w:b:l:e:n:i:v:f:o:m:H:P:")) != EOF) {
		switch (option) {
		case 'b':
			basedir = optarg;
			break;
		case 'c':
			config= optarg;
			break;
		case 'd':
			debug = optarg;
			break;
		case 'e':
			elogfile = optarg;
			break;
		case 'h':
			hostname = optarg;
			break;
		case 'I':
			logfile = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 's':
			server = optarg;
			break;
		case 't':
			thread_count = atoi(optarg);
			break;
		case 'u':
			user = optarg;
			break;
		case 'w':
			socket_timeout = (atoi(optarg) == 0 ? 10 : atoi(optarg)) * 1000
					* 1000;
			break;
		case 'x':
			pass= optarg;
			break;
		case 'n':
			nslice = (atoi(optarg) == 0 ? 60*24 : atoi(optarg));
			break;
		case 'i':
			tslice = (atoi(optarg) == 0 ? 60 : atoi(optarg));
			break;
#ifdef HAVE_LUA
		case 'm':
			mapper_lua = optarg;
			break;
		case 'f':
			input_filter = optarg;
			break;
		case 'o':
			output_filter = optarg;
			break;
		case 'l':
			preload_lua = optarg;
			break;
#else
			case 'm':
			case 'f':
			case 'o':
			case 'l':
			fprintf(stdout, "LUA support unavailable on this copy of dbslayer.");
			return 1;
			break;

#endif
#ifdef HAVE_APR_MEMCACHE_H
		case 'H':
			memcache_host = optarg;
		case 'P':
			memcache_port = (apr_port_t) atoi(optarg);
			break;
#else
			case 'H':
			case 'P':
			fprintf(stdout, "memcache support unavailable on this copy of dbslayer.");
			return 1;
			break;
#endif				
		case 'v':
			fprintf(stdout,"%s",SLAYER_SERVER);
			return 0;
		default:
			break;

		}
	}

	if (server == NULL || config == NULL || thread_count == 0 || port == 0) {
		fprintf(stdout,"Usage %s:  -s server[:server]* -c config \n\t[-u username -x password -t thread-count -p port -h ip-to-bind-to -d debug -w socket-timeout -b basedir -I logfile -e error-logfile -n number-of-stats-buckets [defaults to 1 bucket per minute for 24 hours] -i interval-to-update-stats-buckets [ defaults to 60 seconds]] -f lua-input-filter [lua script against incoming json] -o lua-output-filter [lua script to transfom outgoing json -l lua-preload [lua script for each thread to preload, for common libs and includes] -m [mapping lua] -H [memcached server] -P [mecached port] -v [prints version and exits] \n",basename(argv[0]));
		return 0;
	}

	status = apr_initialize();
	status = apr_pool_create(&mpool,NULL);
	mysql_library_init(0,NULL,NULL);
	reldir = getcwd(NULL,0);

	if (config[0] !='/') {
		config = apr_pstrcat(mpool, reldir, "/", config, NULL);
	}
	if (logfile != NULL && logfile[0] !='/') {
		logfile = apr_pstrcat(mpool, reldir, "/", logfile, NULL);
	}
	if (elogfile != NULL && elogfile[0] !='/') {
		elogfile = apr_pstrcat(mpool, reldir, "/", elogfile, NULL);
	}

	if (debug == NULL) {
		apr_proc_detach(APR_PROC_DETACH_DAEMONIZE);
	}

	chdir("/tmp"); // so I can dump core someplace that I am likely to have write access to 

	apr_queue_create(&out_queue, thread_count, mpool);
	apr_queue_create(&in_queue, thread_count*2, mpool);

	threads = apr_array_make(mpool, thread_count, sizeof(apr_thread_t*));
	apr_threadattr_create(&thread_attr, mpool);
	apr_threadattr_detach_set(thread_attr, 0); // don't detach
	apr_threadattr_stacksize_set(thread_attr, 4096*10);

	td_shared.serialized_schema = NULL;
	td_shared.serialized_mapping = NULL;
	td_shared.in_queue = out_queue;
	td_shared.out_queue = in_queue;
	td_shared.user = user;
	td_shared.pass = pass;
	td_shared.server = server;
	td_shared.config = config;
	td_shared.shutdown = 0;
	td_shared.basedir = basedir;
#ifdef HAVE_LUA
	if (debug) {
		printf(
				"input filter lua file '%s', output filter lua file '%s', preload lua file '%s', mapping lua file '%s' \n",
				input_filter, output_filter, preload_lua, mapper_lua);
	}
	td_shared.mapper_lua = mapper_lua;
	td_shared.input_filter_lua = input_filter;
	td_shared.output_filter_lua = output_filter;
#endif 

	//
	//100 was intended to be a command line argument but number of commandline args is growing out of control
	dbslayer_log_open(&(td_shared.lmanager), logfile, 100, NULL);
	dbslayer_log_open(&(td_shared.elmanager), elogfile, 100, NULL);
	td_shared.stats = dbslayer_stat_init(mpool, nslice, tslice);
	td_shared.startup_args = NULL;
	for (i = 0; i < argc; i++) {
		if (td_shared.startup_args == NULL) {
			td_shared.startup_args = apr_pstrcat(mpool, argv[i], NULL);
		} else {
			td_shared.startup_args = apr_pstrcat(mpool, td_shared.startup_args,
					" ", argv[i], NULL);
		}
	}
#ifdef HAVE_APR_MEMCACHE_H
	if (memcache_host != NULL) {
		rv = apr_memcache_create(mpool, 16, 0, &td_shared.memcache);
		rv = apr_memcache_server_create(mpool, memcache_host, memcache_port, 0,
				thread_count, (thread_count * 2), 
				APR_MEMCACHE_DEFAULT_TTL, &td_shared.memcache_server);
		rv = apr_memcache_add_server(td_shared.memcache,
				td_shared.memcache_server);
	} else {
		td_shared.memcache = NULL;
		td_shared.memcache_server = NULL;
	}
#endif

	create_threads(threads, thread_count, thread_attr, &td_shared, preload_lua,
			mpool);

	//create a stats timer thread
	dbslayer_stats_timer_thread(mpool, td_shared.stats);

	sigset(SIGPIPE,SIG_IGN);
	status = apr_socket_create(&conn, APR_INET, SOCK_STREAM, APR_PROTO_TCP,
			mpool);
	status = apr_socket_opt_set(conn, APR_SO_REUSEADDR, 1);
	status = apr_socket_opt_set(conn, APR_SO_NONBLOCK, 1);
	status = apr_sockaddr_info_get(&addr, hostname ? hostname : APR_ANYADDR, 
	APR_UNSPEC, port, APR_IPV4_ADDR_OK, mpool);
	status = apr_socket_bind(conn, addr);
	if (status != APR_SUCCESS) {
		fprintf(stderr,"couldn't bind to %s:%d\n",hostname ? hostname: "*", port);
		exit(-1);
	}
	status = apr_socket_listen(conn, 3);
	status = apr_pollset_create(&pollset, 1, mpool, 0);
	pfd.p = mpool;
	pfd.desc_type = APR_POLL_SOCKET;
	pfd.reqevents = APR_POLLIN;
	pfd.rtnevents = 0;
	pfd.desc.s = conn;
	pfd.client_data = NULL;
	status = apr_pollset_add(pollset, &pfd);

	printf("Ready to serve requests .... \n");
	serve_requests(conn, ebuf, threads, thread_count, thread_attr, &td_shared,
			preload_lua, out_queue, in_queue, pollset, reldir, socket_timeout,
			mpool);


	dbslayer_log_close(td_shared.lmanager);
	dbslayer_log_close(td_shared.elmanager);
	mysql_library_end();
	apr_pool_destroy(mpool);
	apr_terminate();
	free(reldir);
	return 0;
}
