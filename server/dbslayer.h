#ifndef _QSERVER_H_
#define _QSERVER_H_

#include <apr_general.h>
#include <apr_network_io.h>
#include <apr_strings.h>
#include <apr_uri.h>

/* $Id: dbslayer.h,v 1.2 2007/05/09 20:55:00 derek Exp $ */

typedef struct _manager_t { 
	char *user;
	char *pass;
	char *server;
	char *config;
} manager_t;


#endif
