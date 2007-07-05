noinst_LIBRARIES = libsimplejson.a

AM_CFLAGS = $(MYSQL_CFLAGS) $(APR_CFLAGS) $(APU_CFLAGS)
INCLUDES = -I@top_srcdir@/include $(MYSQL_INCLUDES) $(APR_INCLUDES) $(APU_INCLUDES)
SRC=simplejson.c jsonhelper.c  urldecode.c serializejson.c slayer_utf8.c

libsimplejson_a_SOURCES = $(SRC)
#libsimplejson_a_LIBADD = $(APR_LIBS) $(APU_LIBS) $(MYSQL_LIBS) 

