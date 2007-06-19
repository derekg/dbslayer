# $Id: Makefile,v 1.2 2007/05/04 20:49:08 derek Exp $

include ../make.rules

SRC=simplejson.c jsonhelper.c  urldecode.c serializejson.c slayer_utf8.c

all: libsimplejson.a driver

libsimplejson.a: $(OBJS)
	$(AR)  ru $(LIBDIR)/$@ $(OBJS) 

driver: libsimplejson.a $(OBJDIR)/driver.o
	gcc $(OBJDIR)/driver.o   -lsimplejson $(LDIR) $(LDFLAGS) $(APRUI_LDIR)  -o $(BINDIR)/driver

clean:
	rm -f  $(OBJS) core $(LIBDIR)/libsimplejson.a $(BINDIR)/driver

