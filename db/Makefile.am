noinst_LIBRARIES = libdbaccess.a

AM_CFLAGS = $(MYSQL_CFLAGS) $(APR_CFLAGS) $(APU_CFLAGS)
INCLUDES = -I@top_srcdir@/include $(MYSQL_INCLUDES) $(APR_INCLUDES) $(APU_INCLUDES)
SRC = dbaccess.c

libdbaccess_a_SOURCES = $(SRC)

