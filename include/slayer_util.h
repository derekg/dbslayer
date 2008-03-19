#ifndef _SLAYER_UTIL_H_
#define _SLAYER_UTIL_H_
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/timeb.h>

#include <apr_general.h>
#include <apr_file_io.h>
#include <apr_strings.h>
#include <apr_lib.h>
#include <apr_hash.h>

#include "json_skip.h"
/* $Id: slayer_util.h,v 1.3 2008/02/29 00:18:53 derek Exp $ */
char * urldecode(apr_pool_t *mpool, char *in);
json_skip_head_t * parse_qstring(apr_pool_t *mpool,const char *qstring);

#endif  //_SLAYER_UTIL_H_
