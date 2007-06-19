#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "slayer_util.h"

/* $Id: urldecode.c,v 1.2 2007/05/09 20:55:00 derek Exp $ */

char * urldecode(apr_pool_t *mpool, char *in) { 
		int i;
		char *out,*rout; 

		out = rout = apr_pcalloc(mpool, strlen(in ? in: "" )+1 /* * sizeof(char)*/);
		while(in != NULL && *in !='\0') { 
			if(*in == '%' && *(in+1) !='\0' && *(in+2)!='\0' ) { 
					for(i=0, in++ ; i < 2; i++,in++) {
						(*out) += ((isdigit(*in) ?  *in - '0' : ((tolower(*in) - 'a')) + 10) *  ( i ? 1 : 16));
					}
					out++;
			} else if (*in == '+') {
				*out++ = ' ';
				in++;
			} else {
				*out++ = *in++;
			}
		}
		return  rout;
}
