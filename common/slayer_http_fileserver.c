#include "slayer_http_fileserver.h"
#include <limits.h>
#include <stdlib.h>

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
		char resolved[PATH_MAX];
		char resolved_base[PATH_MAX];
		apr_size_t baselen;
		query = strcmp(query,"/") == 0 ? "/index.html": query;
		char *out = apr_pstrcat(client->request->mpool,server->basedir, query,NULL);

		/** the ".." test above is lexical only - it says nothing about symlinks. resolve
		    both paths and require the target to sit under the base directory. **/
		if(realpath(server->basedir,resolved_base) == NULL) goto not_found;
		if(realpath(out,resolved) == NULL) goto not_found;
		baselen = strlen(resolved_base);
		if(strncmp(resolved,resolved_base,baselen) != 0) goto not_found;
		if(resolved[baselen] != '/' && resolved[baselen] != '\0') goto not_found; //"/srv/wwwevil"

		status = apr_stat(&outfile_stat,resolved,APR_FINFO_SIZE|APR_FINFO_TYPE,client->request->mpool);
		if(status != APR_SUCCESS) goto not_found;
		if(outfile_stat.filetype != APR_REG) goto not_found; //no directories, FIFOs or devices
		status = apr_file_open(&outfile,resolved,APR_READ,APR_OS_DEFAULT,client->request->mpool);
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
			char *output = apr_pcalloc(client->request->mpool,output_size + 1);
			if(output_size > 0) {
				apr_file_read(outfile,output,&output_size);
			}
			apr_file_close(outfile);
			client->request->response_code = 200;
			slayer_http_handle_response(server, client,content_type,output, output_size);
			return NULL;
		}
	} 
not_found:
	client->request->response_code = 404;
	slayer_http_handle_response(server, client, SLAYER_MT_TEXT_PLAIN,"Not Found",-1);
	return NULL;
}
