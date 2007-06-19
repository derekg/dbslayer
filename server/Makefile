# $Id: Makefile,v 1.4 2007/05/09 20:55:00 derek Exp $

include ../make.rules

SRC=dbslayer.c dbslayer_logging.c dbslayer_stats.c
INC+=$(MYSQLINC)

all: dbslayer

dbslayer: $(OBJS)
	$(CC)  $(OBJS) $(LDFLAGS) -ldbaccess -lsimplejson  $(MYSQLLD) $(APRUI_LDIR) $(LDIR) -lm -lz -o $(BINDIR)/$@

clean:
	rm -f  $(OBJS) core $(BINDIR)/dbslayer

