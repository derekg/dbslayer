#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include "slayer_http_server.h"
#include "slayer_http_fileserver.h"




static apr_status_t slayer_http_request_dispatch(slayer_http_server_t *server, slayer_http_connection_t *connection, void **tl_config);

static char * slayer_http_response_code_lookup(int code) {
	char *description = "500 Internal Server Error";
	switch(code) { 
		case 200: description = "200 OK"; break;
		case 403: description = "403 Forbidden"; break;
		case 404: description = "404 Not Found"; break;
		default:
			description = "500 Internal Server Error";
	}
	return description;
}

int slayer_http_handle_response(slayer_http_server_t *server, slayer_http_connection_t *client, const char *mime_type, const char *message, int message_size){
	apr_status_t status;
	char dstring[APR_RFC822_DATE_LEN];
	apr_rfc822_date(dstring,apr_time_now());
	if(message_size == -1) message_size = strlen(message);
	char *header_response = slayer_http_response_code_lookup(client->request->response_code);
	char *header = apr_pstrcat(client->request->mpool, "HTTP/1.0 ",header_response, "\r\n",
	                                   "Date: ",dstring,"\r\n",
	                                   "Server: ",server->server_name,"\r\n",
	                                   "Content-type: ", mime_type, "\r\n",
	                                   "Content-Length: ",apr_itoa(client->request->mpool,client->request->payload_size = message_size),"\r\n",
	                                   "Connection: close\r\n",
	                                   "\r\n",NULL);

	int size; 
	int	header_size = strlen(header); 
	client->request->message_marker = client->request->message = apr_palloc(client->request->mpool, (size = header_size + message_size));
	client->request->message_end = client->request->message_marker + size;
	memcpy(client->request->message,header,header_size);
	memcpy(client->request->message + header_size, message, message_size);
	while ((status = apr_queue_push(server->out_queue,client)) == APR_EINTR);
	return status;
}

static int request_parse(apr_socket_t *conn, slayer_http_request_parse_t *http_request) {
	apr_status_t conn_status;
	int parse_status;

	if (http_request->buffer_marker == NULL || http_request->buffer_marker >= http_request->buffer+http_request->buffer_size) {
		conn_status = apr_socket_recv(conn,http_request->buffer,&(http_request->buffer_size));
		if (conn_status !=APR_SUCCESS || http_request->buffer_size == 0) return -1;
		http_request->buffer_marker = http_request->buffer;
	}
	if (http_request->request_state != PARSE_REQUEST_DONE) {
		parse_status = slayer_http_request_line_parse(http_request);
	}
	if (http_request->request_state == PARSE_REQUEST_DONE) {
		do {
			parse_status = slayer_http_request_header_parse(http_request);
		} while ( (http_request->buffer_marker < http_request->buffer + http_request->buffer_size) && http_request->header_state != PARSE_HEADER_DONE);
	}
	return parse_status;
}


int handle_incoming_connections(slayer_http_server_t *server) {

	apr_socket_t *conn;
	apr_sockaddr_t *addr;
	apr_pollset_t *pollset;
	apr_pollfd_t pfd_listen;
	apr_status_t status;

	const apr_pollfd_t *ret_pfd;
	int events;
	int connections_count = 0;
	slayer_http_connection_t *connections[500];

	//sigset(SIGPIPE,SIG_IGN);
	status = apr_socket_create(&conn,APR_INET,SOCK_STREAM,APR_PROTO_TCP,server->mpool);
	status = apr_socket_opt_set(conn,APR_SO_REUSEADDR,1);
	status = apr_socket_opt_set(conn,APR_SO_NONBLOCK,1);
	status = apr_sockaddr_info_get(&addr,server->hostname ? server->hostname : APR_ANYADDR,APR_UNSPEC,server->port,APR_IPV4_ADDR_OK,server->mpool);
	status = apr_socket_bind(conn,addr);
	if (status != APR_SUCCESS) {
		fprintf(stderr,"couldn't bind to %s:%d\n",server->hostname ? server->hostname: "*", server->port);
		exit(-1);
	}
	status = apr_socket_listen(conn,3);
	status = apr_pollset_create(&pollset,sizeof(connections)/sizeof(slayer_http_connection_t*) +1 /* +1 for listenting socket */,server->mpool,0);
	memset(connections,0,sizeof(connections) );
	pfd_listen.p = server->mpool;
	pfd_listen.desc_type  = APR_POLL_SOCKET;
	pfd_listen.reqevents = APR_POLLIN;
	pfd_listen.rtnevents = 0;
	pfd_listen.desc.s = conn;
	pfd_listen.client_data = NULL;
	status = apr_pollset_add(pollset,&pfd_listen);
	while (1) {
		do {
			status = apr_pollset_poll(pollset, server->socket_timeout , &events, &ret_pfd);
			if (apr_atomic_read32(&(server->shutdown))) return 0;
			//update_alsotry(td_shared->alsotry,td_shared->config); -- NOT SURE WHAT TO ABOUT THIS ONE -  TODO: add additional function
		} while (status == APR_TIMEUP);
		if (status == APR_SUCCESS) {
			int i;
			for ( i = 0; i < events; i++) {
				if (ret_pfd[i].client_data == NULL) {
					if (connections_count < (sizeof(connections) / sizeof(slayer_http_connection_t*)) -1) {
						apr_pool_t *cmpool,*rmpool;
						int j;
						slayer_http_connection_t *connection;
						apr_pool_create(&cmpool,NULL);
						connection = apr_pcalloc(cmpool,sizeof(slayer_http_connection_t));
						connection->mpool = cmpool;
						apr_pool_create(&rmpool,connection->mpool);
						connection->request = apr_pcalloc(rmpool,sizeof(slayer_http_request_t));
						connection->request->mpool = rmpool;
						slayer_http_request_parse_init(connection->request->mpool,&(connection->request->parse),server->uri_size);
						connection->request->begin_request = apr_time_now();

						//setup the socket
						status = apr_socket_accept(&(connection->conn),conn,connection->mpool);
						status = apr_socket_opt_set(connection->conn,APR_SO_NONBLOCK,1);
						memset(&connection->pollfd,0,sizeof(apr_pollfd_t));

						connection->pollfd.client_data = connection;
						connection->pollfd.desc_type = APR_POLL_SOCKET;
						connection->pollfd.desc.s = connection->conn;
						connection->pollfd.reqevents = APR_POLLIN;
						connections_count++;
						apr_pollset_add(pollset,&(connection->pollfd));

						for (j = 0; j <  (sizeof(connections) / sizeof(slayer_http_connection_t*));j++) {
							if (connections[j] == NULL) {
								connections[j] = connection;
								break;
							}
						}
					} else {
						//do nothing ? should back up and be auto rejected.
					}
				} else {
					slayer_http_connection_t *connection= (slayer_http_connection_t*)ret_pfd[i].client_data;
					if (ret_pfd[i].rtnevents & APR_POLLERR ) {
						connection->request->done = 1;
					} else if ( ret_pfd[i].rtnevents & APR_POLLHUP) {
						connection->request->done = 1;
					} else if ( ret_pfd[i].rtnevents & APR_POLLIN || ret_pfd[i].rtnevents & APR_POLLPRI) {
						if (request_parse(connection->conn,connection->request->parse) == -1) {
							connection->request->done = 1;
						} else if (connection->request->parse->header_state == PARSE_HEADER_DONE) {
							connection->request->read_done = 1;
						}
					} else {
						printf("oddball case\n");
						//print handle oddball case
					}
				}
			}//end of for loop
			for (i = 0; i < sizeof(connections) /sizeof(slayer_http_connection_t*); i++) {
				if (connections[i] != NULL  && connections[i]->request != NULL) {
					if (connections[i]->request->read_done) {
						apr_pollset_remove(pollset,&(connections[i]->pollfd));
						while ((status == apr_queue_push(server->in_queue,connections[i])) == APR_EINTR);
						if (status == APR_EOF) return 0;
						connections[i] = NULL;
						connections_count--;
					} else if (connections[i]->request->done || apr_time_as_msec(apr_time_now() - connections[i]->request->begin_request) > 10000) {
						apr_pollset_remove(pollset,&(connections[i]->pollfd));
						apr_socket_close(connections[i]->conn);
						apr_pool_destroy(connections[i]->mpool);
						connections[i] = NULL;
						connections_count--;
					}
				}
			}
		} else {
			char ebuf[1024];
			char *err_msg = apr_strerror(status,ebuf,sizeof(ebuf));
			fprintf(stderr,"ERROR in apr_pollset_poll - %s\n",err_msg);
			slayer_server_log_message(server->elmanager,"ERROR in apr_pollset_poll - ");
			slayer_server_log_message(server->elmanager,err_msg);
			slayer_server_log_message(server->elmanager,"\n");
		}
	}
	return 0;
}

void* handle_running_request(apr_thread_t *mythread,void * _server) {
	void *_connection;
	void **tl_config;
	int i =0;
	apr_pool_t *mpool;
	apr_status_t status = apr_pool_create(&mpool,NULL);
	slayer_http_server_t *server= (slayer_http_server_t*) _server;
	tl_config = apr_pcalloc(mpool,sizeof(void *) * server->service_map_size);
	// ****** initialize in thread specific calls for the handlers *************
	for(i = 0; i < server->service_map_size; i++) {
		tl_config[i] = server->service_map[i]->service->service_thread_init_func !=NULL ? server->service_map[i]->service->service_thread_init_func(mpool,server->service_map[i]->global_config) : NULL;
	}
	do {
		_connection = NULL;;
		while ( (status = apr_queue_pop(server->in_queue,&_connection)) == APR_EINTR);
		if (status == APR_SUCCESS) {
			slayer_http_connection_t *connection = (slayer_http_connection_t*) _connection;
			status = slayer_http_request_dispatch(server,connection,tl_config);
			if (status != APR_SUCCESS) {
				apr_queue_term(server->out_queue);
				apr_queue_term(server->in_queue);
				apr_atomic_set32(&(server->shutdown),1);
			}
		}
	} while ( status == APR_SUCCESS);
	//clean it all up at the end
	for(i = 0; i < server->service_map_size; i++) {
		if(server->service_map[i]->service->service_thread_destroy_func){ server->service_map[i]->service->service_thread_destroy_func(tl_config[i]); }
	}
	apr_pool_destroy(mpool);
	return NULL;
}
//
//http_output_response() - 
//thread that listens on the queue | outgoing_thread_run
void* handle_outgoing_response(apr_thread_t *mythread,void * _server) {
	slayer_http_connection_t *connections[500];
	int connection_count=0;
	void *fetch = NULL;
	apr_pool_t *mpool;
	apr_pollset_t *pollset;

	apr_status_t status = apr_pool_create(&mpool,NULL);
	slayer_http_server_t *server  = (slayer_http_server_t*) _server;

	status = apr_pollset_create(&pollset,sizeof(connections)/sizeof(slayer_http_connection_t*),mpool,0);
	memset(connections,0,sizeof(connections) );

	//setup the pollset
	do {
		fetch = NULL;
		if (connection_count > 0) {
			const apr_pollfd_t *ret_pfd;
			int events,i;
			events =0;
			status = apr_pollset_poll(pollset, 1000*500 /* .05 sec */ , &events, &ret_pfd);
			for (i= 0; i < events; i++) {
				slayer_http_connection_t *connection = (slayer_http_connection_t*) ret_pfd[i].client_data;
				if (ret_pfd[i].rtnevents & APR_POLLHUP || ret_pfd[i].rtnevents & APR_POLLERR) {
					char *http_request = apr_pstrcat(connection->request->mpool,connection->request->parse->method == HTTP_METHOD_GET ? "GET ": "POST ",connection->request->parse->uri_data,connection->request->parse->version == HTTP_10 ? " HTTP/1.0" : " HTTP/1.0",NULL);
					slayer_server_log_err_message(server->elmanager,connection->request->mpool,connection->conn,http_request,"Connection closed by client");
					connection->request->done =1;
				} else if (ret_pfd[i].rtnevents &  APR_POLLOUT) {
					apr_size_t bsent = (connection->request->message_end - connection->request->message_marker);
					status = apr_socket_send(connection->conn,connection->request->message_marker ,&bsent);
					if (status != APR_SUCCESS || bsent == 0) {
						connection->request->done = 1;
						char *http_request = apr_pstrcat(connection->request->mpool,connection->request->parse->method == HTTP_METHOD_GET ? "GET ": "POST ",connection->request->parse->uri_data,connection->request->parse->version == HTTP_10 ? " HTTP/1.0" : " HTTP/1.0",NULL);
						slayer_server_log_err_message(server->elmanager,connection->request->mpool,connection->conn,http_request,"Connection closed by client");
					} else if (bsent > 0) {
						connection->request->message_marker += bsent;
						if (connection->request->message_marker == connection->request->message_end) {
							connection->request->done =1;
							char *http_request = apr_pstrcat(connection->request->mpool,connection->request->parse->method == HTTP_METHOD_GET ? "GET ": "POST ",connection->request->parse->uri_data,connection->request->parse->version == HTTP_10 ? " HTTP/1.0" : " HTTP/1.0",NULL);
							slayer_server_stat_update(server->stats);
							slayer_server_log_request(server->lmanager,connection->request->mpool,connection->conn,http_request,connection->request->response_code,connection->request->payload_size,apr_time_now() - connection->request->begin_request);
						}
					}
				}
			}
			for (i = 0; connection_count > 0 && i < sizeof(connections) /sizeof(slayer_http_connection_t*); i++) {
				if (connections[i] != NULL && (connections[i]->request->done || apr_time_as_msec(apr_time_now() - connections[i]->request->begin_request) > 10000)) {
					apr_pollset_remove(pollset,&(connections[i]->pollfd));
					apr_socket_close(connections[i]->conn);
					apr_pool_destroy(connections[i]->mpool);
					connections[i] = NULL;
					connection_count--;
				}
			}
			status = apr_queue_trypop(server->out_queue,&fetch);
		} else {
			status = apr_queue_pop(server->out_queue,&fetch);
		}
		if (status == APR_SUCCESS) {
			int i;
			slayer_http_connection_t *connection = (slayer_http_connection_t*) fetch;
			for (i = 0; i < sizeof(connections) / sizeof(slayer_http_connection_t*); i++) {
				if (connections[i] == NULL) {
					connections[i] = connection;
					connection->pollfd.reqevents = APR_POLLOUT;
					apr_pollset_add(pollset,&(connection->pollfd));
					connection_count++;
					break;
				}
			}
			if (i == sizeof(connections) / sizeof(slayer_http_connection_t*)) {
				apr_socket_close(connection->conn);
				apr_pool_destroy(connection->mpool);
			}
		} else if ( status == APR_EOF) {
			goto terminate;
		}
	} while (1);
terminate:
	apr_pool_destroy(mpool);
	return NULL;
}

static apr_status_t slayer_http_request_dispatch(slayer_http_server_t *server, slayer_http_connection_t *client, void **tl_config) {
	apr_status_t status;
	slayer_http_request_parse_t *parse= client->request->parse;
	int i;

	apr_uri_parse(client->request->mpool,parse->uri_start,&(client->request->uri));
	if (client->request->uri.path == NULL) {
		apr_socket_close(client->conn);
		apr_pool_destroy(client->mpool);
		return APR_SUCCESS;
	}
	int nothandled = 1;
	for(i = 0; nothandled && i < server->service_map_size ; i++) { 
		int j;
		for(j = 0; nothandled && server->service_map[i]->urls[j] !=NULL ; j++) { 
			if(strcmp(client->request->uri.path,server->service_map[i]->urls[j]) == 0) { 
				nothandled = 0;
				server->service_map[i]->service->service_handler_func(server,server->service_map[i]->global_config,client,tl_config[i]);
			}
		}	
	}
	if(nothandled) {
		apr_sockaddr_t *local_addr;
		apr_sockaddr_t *remote_addr;
		char *local_ip;
		char *remote_ip;
		char *message =NULL;
		apr_socket_addr_get(&remote_addr,APR_REMOTE,client->conn);
		apr_sockaddr_ip_get(&remote_ip,remote_addr);
		apr_socket_addr_get(&local_addr,APR_LOCAL,client->conn);
		apr_sockaddr_ip_get(&local_ip,local_addr);

		if ( strcmp(client->request->uri.path,"/shutdown") == 0 ) {
			if (strcmp(local_ip,remote_ip) != 0) {
				client->request->response_code = 403;	
				slayer_http_handle_response(server, client, SLAYER_MT_TEXT_PLAIN,"You are not authorized!",-1);
			} else { 
				client->request->response_code = 200;	
				slayer_http_handle_response(server, client ,SLAYER_MT_TEXT_PLAIN,"going down",-1);
				status = apr_socket_close(client->conn);
				apr_pool_destroy(client->mpool);
				return APR_EOF;
			}
		} else  if (strcmp(client->request->uri.path,"/stats") == 0) {
			client->request->response_code = 200;	
			message = slayer_server_stats_tojson(server->stats,client->request->mpool);
			slayer_http_handle_response(server, client, SLAYER_MT_TEXT_PLAIN,message,-1);
		} else  if (strcmp(client->request->uri.path,"/stats/log") == 0) {
			client->request->response_code = 200;	
			message = slayer_server_log_get_entries(server->lmanager,client->request->mpool);
			slayer_http_handle_response(server, client, SLAYER_MT_TEXT_PLAIN,message,-1);
		} else  if (strcmp(client->request->uri.path,"/stats/error") == 0) {
			client->request->response_code = 200;	
			message = slayer_server_log_get_entries(server->elmanager,client->request->mpool);
			slayer_http_handle_response(server, client, SLAYER_MT_TEXT_PLAIN,message, -1);
		} else if (strcmp(client->request->uri.path,"/stats/args") == 0) {
			client->request->response_code = 200;	
			slayer_http_handle_response(server, client, SLAYER_MT_TEXT_PLAIN,server->startup_args,-1);
		} else {
			slayer_http_fileserver(server,client);
		}
	}
	return APR_SUCCESS;
}

static void slayer_server_parse_args(int argc, char **argv,slayer_http_server_t *server) { 
	int i;
	for(i = 1; i < argc; i++) { 	
		if(argv[i][0] == '-' && strlen(argv[i]) == 2) {
			char *extra_arg = i+1 < argc && argv[i+1][0] != '-' ? argv[i+1] : NULL;
		switch (argv[i][1]) {
		  case '?':
			  fprintf(stdout,"Usage %s:  [-t thread-count -p port -h ip-to-bind-to -d debug -w socket-timeout -b basedir -l logfile -e error-logfile -n number-of-stats-buckets [defaults to 1 bucket per minute for 24 hours] -i interval-to-update-stats-buckets [ defaults to 60 seconds] ] -v [prints version and exits]\n",basename(argv[0]));
			  for(i = 0; i < server->service_map_size; i++) { 	
				  fprintf(stdout,"\t %s\n",server->service_map[i]->service->help_string);
			  }
			  exit(0);
		  case 'b':
			  server->basedir = extra_arg;
			  break;
		  case 'd':
			  server->debug = extra_arg;
			  break;
		  case 'e':
			  server->elogfile = extra_arg;
			  break;
		  case 'h':
			  server->hostname = extra_arg;
			  break;
		  case 'l':
			  server->logfile = extra_arg ;
			  break;
		  case 'p':
			  server->port = atoi(extra_arg);
			  break;
		  case 't':
			  server->thread_count = atoi(extra_arg);
			  break;
		  case 'w':
			  server->socket_timeout = (atoi(extra_arg) == 0 ? 10 : atoi(extra_arg)) * 1000 * 1000;
			  break;
		  case 'n':
			  server->nslice = (atoi(extra_arg) == 0 ? 60*24 : atoi(extra_arg)) ;
			  break;
		  case 'i':
			  server->tslice = (atoi(extra_arg) == 0 ? 60 : atoi(extra_arg)) ;
			  break;
		  case 'v':
			  fprintf(stdout,"%s\n",server->server_name);
			  exit(0);
		  default:
			  break;
		  }	//end of switch
		} //if 
	}
	if ( server->thread_count == 0 || server->port == 0 ) {
		fprintf(stdout,"Usage %s:  [-t thread-count -p port -h ip-to-bind-to -d debug -w socket-timeout -b basedir -l logfile -e error-logfile -n number-of-stats-buckets [defaults to 1 bucket per minute for 24 hours] -i interval-to-update-stats-buckets [ defaults to 60 seconds] ] -v [prints version and exits]\n",basename(argv[0]));
		for(i = 0; i < server->service_map_size; i++) { 	
			fprintf(stdout,"\t %s\n",server->service_map[i]->service->help_string);
		}
		exit(0);
	}
}//end of parseargs

int slayer_server_run(int service_map_size, slayer_http_service_map_t **service_map, int argc, char **argv,int uri_size, const char *server_name) {
	apr_status_t status;
	apr_threadattr_t *thread_attr;
	apr_array_header_t *threads;
	int i ;
	char *reldir = NULL;
	slayer_http_server_t server;

	memset(&server,0,sizeof(slayer_http_server_t));
	server.port = 9090;
	server.socket_timeout =  2 * 1000* 1000;
	server.nslice = 60 * 24;
	server.tslice = 60 ;
	server.service_map = service_map;
	server.service_map_size = service_map_size;
	server.uri_size = uri_size;
	server.thread_count = 1;
	server.server_name = server_name;

	slayer_server_parse_args(argc,argv,&server);

	server.thread_count++;

	status = apr_initialize();
	status = apr_pool_create(&(server.mpool),NULL);
	reldir = getcwd(NULL,0);

	if (server.config !=NULL && server.config[0] !='/') {
		server.config = apr_pstrcat(server.mpool,reldir,"/",server.config,NULL);
	}
	if (server.logfile !=NULL && server.logfile[0] !='/') {
		server.logfile = apr_pstrcat(server.mpool,reldir,"/",server.logfile,NULL);
	}
	if (server.elogfile !=NULL && server.elogfile[0] !='/') {
		server.elogfile = apr_pstrcat(server.mpool,reldir,"/",server.elogfile,NULL);
	}

	chdir("/tmp"); // so I can dump core someplace that I am likely to have write access to

	//call the global initializers
	for(i = 0; i < server.service_map_size; i++) { 
		if(server.service_map[i]->service->service_global_init_func) server.service_map[i]->global_config = server.service_map[i]->service->service_global_init_func(server.mpool,argc,argv);
	}

	if (server.debug == NULL) {
		apr_proc_detach(APR_PROC_DETACH_DAEMONIZE);
	}

	apr_queue_create(&(server.out_queue),server.thread_count,server.mpool);
	apr_queue_create(&(server.in_queue),server.thread_count*2,server.mpool);

	threads = apr_array_make(server.mpool,server.thread_count,sizeof(apr_thread_t*));
	apr_threadattr_create(&thread_attr,server.mpool);
	apr_threadattr_detach_set(thread_attr,0); // don't detach
	apr_threadattr_stacksize_set(thread_attr,4096*10);

	//
	//100 was intended to be a command line argument but number of commandline args is growing out of control
	slayer_server_log_open(&(server.lmanager),server.logfile,100,NULL);
	slayer_server_log_open(&(server.elmanager),server.elogfile,100,NULL);
	server.stats = slayer_server_stat_init(server.mpool,server.nslice,server.tslice);
	for (i = 0; i < argc; i++) {
		if (server.startup_args == NULL) {
			server.startup_args = apr_pstrcat(server.mpool,argv[i],NULL);
		} else {
			server.startup_args = apr_pstrcat(server.mpool,server.startup_args," ",argv[i],NULL);
		}
	}

	//setup outgoing thread.
	{
		apr_thread_t *thread;
		apr_thread_create(&thread,thread_attr,handle_outgoing_response,&server,server.mpool);
		*((apr_thread_t**)(apr_array_push(threads))) = thread;
	}

	for (i = 1; i < server.thread_count;i++) {
		apr_thread_t *thread;
		apr_thread_create(&thread,thread_attr,handle_running_request,&server,server.mpool);
		*((apr_thread_t**)(apr_array_push(threads))) = thread;
	}

	//create a stats timer thread
	slayer_server_stats_timer_thread(server.mpool,server.stats);

	handle_incoming_connections(&server);

	for (i = 0; i < server.thread_count;i++) {
		apr_thread_t *thread = *((apr_thread_t**)(threads->elts + (threads->elt_size * i)));
		apr_thread_join(&status,thread);
	}

	for(i = 0; i < server.service_map_size; i++) { 
		if(server.service_map[i]->service->service_global_destroy_func)   
			server.service_map[i]->service->service_global_destroy_func(server.service_map[i]->global_config);
	}
	slayer_server_log_close(server.lmanager);
	slayer_server_log_close(server.elmanager);
	apr_pool_destroy(server.mpool);
	apr_terminate();
	free(reldir);
	return 0;
}
