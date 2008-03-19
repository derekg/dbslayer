noinst_LIBRARIES = libsimplejson.a

AM_CFLAGS = $(MYSQL_CFLAGS) $(APR_CFLAGS) $(APU_CFLAGS)
INCLUDES = -I@top_srcdir@/include $(MYSQL_INCLUDES) $(APR_INCLUDES) $(APU_INCLUDES)
SRC=json2xml.c jsonhelper.c json_skip.c qstring_parse.c serializejson.c simplejson.c slayer_http_fileserver.c slayer_http_parse.c slayer_http_server.c slayer_server_logging.c slayer_server_stats.c slayer_utf8.c urldecode.c

libsimplejson_a_SOURCES = $(SRC)
#libsimplejson_a_LIBADD = $(APR_LIBS) $(APU_LIBS) $(MYSQL_LIBS) 

