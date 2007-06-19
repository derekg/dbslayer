# $Id: Makefile,v 1.1.1.1 2007/03/20 22:47:18 derek Exp $

include ../make.rules

SRC=dbaccess.c
INC+=$(MYSQLINC)

all: dbaccess libdbaccess.a

dbaccess: $(OBJS) $(OBJDIR)/dbdriver.o
	$(CC)  $(OBJS) $(OBJDIR)/dbdriver.o -lsimplejson $(LDIR) $(LDFLAGS) \
		$(APR_LDIR) $(MYSQLLD) -lm -lz -o $(BINDIR)/$@

libdbaccess.a: $(OBJS)
	$(AR)  ru $(LIBDIR)/$@ $(OBJS) 

clean:
	rm -f  $(OBJS) core $(BINDIR)/dbaccess $(LIBDIR)/libdbaccess.a

