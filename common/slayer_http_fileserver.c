#include "slayer_http_fileserver.h"

typedef struct _slayer_mtypes_t { 
	char *suffix;
	char *mtype;
} slayer_mtypes_t;

slayer_mtypes_t mtypes[] = {
	{".html", "text/html"},		
	{".png", "image/png"},		
	{".gif", "image/gif"},		
	{".jpg", "image/jpeg"},		
	{".xml", "text/xml"},		
	{".xsl", "text/xml"},		
	{NULL,NULL} 
};

void * slayer_http_fileserver(slayer_http_server_t *server, slayer_http_connection_t *client) { 
	char *query = client->request->uri.path; 
	if(server->basedir && query && strstr(query,"..") == NULL) {
		apr_status_t status;
		apr_file_t *outfile;
		apr_finfo_t outfile_stat;
		query = strcmp(query,"/") == 0 ? "/index.html": query;
		char *out = apr_pstrcat(client->request->mpool,server->basedir, query,NULL);
		status = apr_stat(&outfile_stat,out,APR_FINFO_SIZE,client->request->mpool);
		status = apr_file_open(&outfile,out,APR_READ,APR_OS_DEFAULT,client->request->mpool);
		if(status == APR_SUCCESS) {
			char *end = query + strlen(query);
			while(*end != '.' && end != query) end--;
			char *content_type = "text/plain";
			if( *end == '.')  { 
					int i = 0;
					for(i = 0;  mtypes[i].suffix; i++)   {
						if(strcasecmp(end,mtypes[i].suffix) == 0) { 
							content_type = mtypes[i].mtype;
							break;
						}
					}
			}
			apr_size_t  output_size = outfile_stat.size;
			char *output = apr_pcalloc(client->request->mpool,output_size);
			apr_file_read(outfile,output,&output_size);
			apr_file_close(outfile);
			client->request->response_code = 200;
			slayer_http_handle_response(server, client,content_type,output, output_size);
			return NULL;
		}
	} 
	client->request->response_code = 404;
	slayer_http_handle_response(server, client, SLAYER_MT_TEXT_PLAIN,"Not Found",-1);
	return NULL;
}
