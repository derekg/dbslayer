#!/bin/bash

# $Id: getenv.sh,v 1.1.1.1 2007/03/20 22:47:18 derek Exp $

touch $1

echo -n "Enter Apache Runtime Directory: "
read line
if [ $line ]; then
	echo "APR_BIN=$line" >> $1
fi
