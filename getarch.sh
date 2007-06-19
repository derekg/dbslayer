#!/bin/sh

# $Id: getarch.sh,v 1.2 2007/05/09 20:55:00 derek Exp $

arch=
case `uname -p | tr "[A-Z]" "[a-z]"` in
    i?86)
        arch=x86
    ;;
    sparc)
        arch=sparc
    ;;
    x86_64)
        arch=x86_64
    ;;
esac
echo $arch
