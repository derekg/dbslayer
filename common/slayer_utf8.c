#include "slayer_utf8.h"

/* $Id */

char * slayer_escaped2utf8(apr_pool_t *mpool, unsigned int code)   { 
	char *out = NULL;
	if (code < 0 || code > 0x7FFFFFFF) {
		out = NULL;
	} else if (code < 0x00000080) { 
		out = apr_psprintf(mpool,"%c",code);
	} else if (code < 0x00000800) { 
		out = apr_psprintf(mpool,"%c%c", 
			0xC0 + (code >> 6), 
			0x80 + (code & 0x3F));
	} else if (code < 0x00010000) { 
		out = apr_psprintf(mpool,"%c%c%c",0xE0 + (code >> 12), 
			0x80 + ((code >> 6) & 0x3F), 
			0x80 + (code & 0x3F));
	} else if (code < 0x00200000) {
		out = apr_psprintf(mpool,"%c%c%c%c", 
				0xF0 + (code >> 18), 
				0x80 + ((code >> 12) & 0x3F), 
				0x80 + ((code >> 6) & 0x3F), 
				0x80 + (code & 0x3F));
	} else if (code < 0x04000000) { 
		out = apr_psprintf(mpool,"%c%c%c%c%c",
				0xF8 + (code >> 24),
				0x80 + ((code >> 18) & 0x3F),
				0x80 + ((code >> 12) & 0x3F),
				0x80 + ((code >> 6) & 0x3F),
				0x80 + (code & 0x3F));
	} else { 
		out = apr_psprintf(mpool,"%c%c%c%c%c%c",
			0xFC + (code >> 30),
			 0x80 + ((code >> 24) & 0x3F),
			 0x80 + ((code >> 18) & 0x3F),
			 0x80 + ((code >> 12) & 0x3F),
			 0x80 + ((code >> 6) & 0x3F),
			 0x80 + (code & 0x3F));
	}
	return out;
}

int slayer_hex2int(char *in) { 
	int i;
	int len = strlen(in);
	int result = 0;
	for(i = 0 ;i < len ; i++) {
		if( in[i] >= 'a' && in[i] <= 'f') {
			result += ((len-i -1) > 0 ? pow(16,(len-i -1)): 1) * (in[i] -'a'  + 10); 
		} else if(in[i] >='A' && in[i] <='F'){
			result += ((len-i -1 ) > 0 ? pow(16,(len-i -1)): 1) * (in[i]-'A' + 10); 
		} else if( in[i] >='0' && in[i] <='9') { 
			result += ((len-i - 1) > 0 ? pow(16,(len-i-1)): 1) * (in[i] - '0' ); 
		} else {
			return -1;
		}
	}
	return result;
}

