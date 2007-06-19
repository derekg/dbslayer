# $Id: Makefile,v 1.1.1.1 2007/03/20 22:47:18 derek Exp $

OS=$(shell ./getos.sh)
ARCH=$(shell ./getarch.sh)

PLATFORM=$(OS).$(ARCH)

BUILD=common db server

all: 
	mkdir -p ../$(PLATFORM)/lib ../$(PLATFORM)/obj  ../$(PLATFORM)/bin 
	for dir in $(BUILD); do $(MAKE) -w -C $$dir; done

clean:
	for dir in $(BUILD); do $(MAKE) -w -C $$dir clean; done
