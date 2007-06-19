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

/* $Id: slayer_util.h,v 1.2 2007/05/09 20:55:00 derek Exp $ */

char * urldecode(apr_pool_t *mpool, char *in);

#endif  //_SLAYER_UTIL_H_
