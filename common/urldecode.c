#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "slayer_util.h"

/* $Id: urldecode.c,v 1.2 2007/05/09 20:55:00 derek Exp $ */

static int hexval(unsigned char c) {
	if (isdigit(c)) return c - '0';
	if (isxdigit(c)) return tolower(c) - 'a' + 10;
	return -1;
}

char * urldecode(apr_pool_t *mpool, char *in) { 
		char *out,*rout; 

		out = rout = apr_pcalloc(mpool, strlen(in ? in: "" )+1 /* * sizeof(char)*/);
		while(in != NULL && *in !='\0') { 
			if(*in == '%' && *(in+1) !='\0' && *(in+2)!='\0' ) { 
					int hi = hexval((unsigned char)*(in+1));
					int lo = hexval((unsigned char)*(in+2));
					if (hi >= 0 && lo >= 0) {
						*out++ = (char)((hi << 4) | lo);
						in += 3;
					} else {
						//not a valid %XX escape - pass the '%' through literally
						*out++ = *in++;
					}
			} else if (*in == '+') {
				*out++ = ' ';
				in++;
			} else {
				*out++ = *in++;
			}
		}
		return  rout;
}
