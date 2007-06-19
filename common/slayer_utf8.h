#ifndef _SLAYER_UTF8_H_
#define _SLAYER_UTF8_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_tables.h>
#include <apr_hash.h>
#include <apr_strings.h>

/* $Id: slayer_utf8.h,v 1.2 2007/05/09 20:55:00 derek Exp $ */

char* slayer_escaped2utf8(apr_pool_t *mpool, unsigned int code);
int slayer_hex2int(char *in);

#endif //_SLAYER_UTF8_H_
