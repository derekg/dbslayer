#ifndef _JSON2XML_H_
#define _JSON2XML_H_
#include "simplejson.h"
char * xml_serialize(apr_pool_t *mpool, json_value *json);
#endif /*_JSON2XML_H_*/
