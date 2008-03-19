bin_PROGRAMS = dbslayer

dbslayer_LDADD = -L@top_srcdir@/common -lsimplejson -L@top_srcdir@/db -ldbaccess $(APU_LIBS) $(APR_LIBS)  $(MYSQL_LIBS) 

AM_CFLAGS = $(MYSQL_CFLAGS) $(APR_CFLAGS) $(APU_CFLAGS) -I@top_srcdir@/include $(MYSQL_INCLUDES) $(APR_INCLUDES) $(APU_INCLUDES)

dbslayer_SOURCES = dbslayer.c dbslayer_logging.c dbslayer_stats.c

