#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.60)
AC_INIT(dbslayer, 1.0)

AM_INIT_AUTOMAKE(dbslayer, 1.0)
AC_CONFIG_SRCDIR([db/dbaccess.c])
AC_CONFIG_SRCDIR([common/urldecode.c])
AC_CONFIG_SRCDIR([server/dbslayer.c])
#AC_CONFIG_HEADER([config.h])

dnl apache run time apr is required
AC_ARG_WITH(apr-1-config, 
[  --with-apr-1-config=PATH   The path to apr-1-config if not in $PATH], [
  APRCONFIG=$withval
], [
  APRCONFIG=apr-1-config-was-not-found-by-configure
])

if test "x$APRCONFIG" = "xapr-1-config-was-not-found-by-configure"; then 
  AC_PATH_PROG( APRCONFIG, apr-1-config, apr-1-config-was-not-found-by-configure,
              $PATH:/usr/bin:/usr/local/bin:/usr/local/apr/bin)
fi

AC_SUBST(APRCONFIG)

if test "x$APRCONFIG" = "xapr-1-config-was-not-found-by-configure"; then
  AC_MSG_ERROR([apr-1-config wasn't found please install http://apr.apache.org ])
else
  APR_CFLAGS=`$APRCONFIG --cflags --cppflags`
  APR_INCLUDES=`$APRCONFIG --includes`
  APR_LIBS=`$APRCONFIG --link-ld --libs`
fi

AC_SUBST(APR_CFLAGS)
AC_SUBST(APR_INCLUDES)
AC_SUBST(APR_LIBS)

AC_MSG_CHECKING(for APR CFLAGS)
AC_MSG_RESULT($APR_CFLAGS)
AC_MSG_CHECKING(for APR includes)
AC_MSG_RESULT($APR_INCLUDES)
AC_MSG_CHECKING(for APR libraries)
AC_MSG_RESULT($APR_LIBS)


dnl apache run time apr utility is required
AC_ARG_WITH(apu-1-config, 
[  --with-apu-1-config=PATH   The path to apu-1-config if not in $PATH], [
  APUCONFIG=$withval
], [
  APUCONFIG=apu-1-config-was-not-found-by-configure
])

if test "x$APUCONFIG" = "xapu-1-config-was-not-found-by-configure"; then 
  AC_PATH_PROG( APUCONFIG, apu-1-config, apu-1-config-was-not-found-by-configure,
              $PATH:/usr/bin:/usr/local/bin:/usr/local/apr/bin)
fi

AC_SUBST(APUCONFIG)
if test "x$APUCONFIG" = "xapu-1-config-was-not-found-by-configure"; then
  AC_MSG_ERROR([apu-1-config wasn't found please install http://apr.apache.org ])
else
  APU_INCLUDES=`$APUCONFIG --includes`
  APU_LIBS=`$APUCONFIG --link-ld --libs`
fi
AC_SUBST(APU_INCLUDES)
AC_SUBST(APU_LIBS)

AC_MSG_CHECKING(for APR Utility includes)
AC_MSG_RESULT($APU_INCLUDES)
AC_MSG_CHECKING(for APR Utility libraries)
AC_MSG_RESULT($APU_LIBS)

dnl apache run time apr is required
AC_ARG_WITH(mysql-config, 
[  --with-mysql-config=PATH   The path to mysql-config if not in $PATH], [
  MYSQLCONFIG=$withval
], [
  MYSQLCONFIG=mysql-config-was-not-found-by-configure
])

if test "x$MYSQLCONFIG" = "xmysql-config-was-not-found-by-configure"; then
	AC_PATH_PROG( MYSQLCONFIG, mysql_config, mysql_config-was-not-found-by-configure,
              $PATH:/usr/bin:/usr/local/bin)
fi

AC_SUBST(MYSQLCONFIG)
if test "x$MYSQLCONFIG" = "xmysql_config-was-not-found-by-configure"; then
  AC_MSG_ERROR([mysql_config wasn't found please install http://www.mysql.com ])
else
  MYSQL_CFLAGS=`$MYSQLCONFIG --cflags`
  MYSQL_INCLUDES=`$MYSQLCONFIG --include`
  MYSQL_LIBS=`$MYSQLCONFIG --libs_r`
fi
AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_INCLUDES)
AC_SUBST(MYSQL_LIBS)



AC_MSG_CHECKING(for MySQL CFLAGS)
AC_MSG_RESULT($MYSQL_CFLAGS)
AC_MSG_CHECKING(for MySQL includes)
AC_MSG_RESULT($MYSQL_INCLUDES)
AC_MSG_CHECKING(for MySQL libraries)
AC_MSG_RESULT($MYSQL_LIBS)
#apr_memcache support
AC_CHECK_LIB(apr_memcache, apr_memcache_create, [APR_MEMCACHE_LIBS='-lapr_memcache'])
AC_CHECK_LIB(apr_memcache, apr_memcache_create, [AC_DEFINE(HAVE_APR_MEMCACHE_H, 1)])
AC_SUBST(APR_MEMCACHE_LIBS)
#AC_CHECK_HEADERS([apr_memcache])
AC_MSG_CHECKING(for apr_memcache)
AC_MSG_RESULT($APR_MEMCACHE_LIBS)
#LUA support
AC_CHECK_LIB(lua, lua_newstate, [LUA_LIBS='-llua '])
AC_MSG_RESULT($LUA_LIBS)
AC_CHECK_HEADERS([lua.h  lualib.h])
AC_CHECK_LIB(lua5.1, lua_newstate, [LUA_LIBS='-llua5.1 -llua5.1-filesystem'])
AC_CHECK_HEADERS([lua5.1/lua.h lua5.1/lualib.h])
AC_MSG_RESULT($LUA_INCLUDES_FOUND)
AC_SUBST(LUA_LIBS)
LUA_INCLUDES="-I lua"
for file in lua/*.lua; do 
    destfile=`basename $file .lua`_parser_lua.h
    define=`basename $destfile .h`
    define=`echo "${define}"`
    echo "char * ${define}() {" > server/$destfile
    cat $file | sed -e 's/\\/\\\\/g' | sed -e 's/"/\\"/g' | tr -d '\015' |  tr -d '\177-\255' | sed -e 's/$/\\n/' | tr -d '\012' | sed -e "s/^/    return(\""/ | sed -e 's/$/");}/g'>> server/$destfile;
    echo "" >> server/$destfile;  
    AC_MSG_RESULT(generating $destfile from $file)
done

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
# AC_PROG_LIBTOOL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-ldbaccess':
#AC_CHECK_LIB([dbaccess], [main])
# FIXME: Replace `main' with a function in `-lm':
#AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lsimplejson':
#AC_CHECK_LIB([simplejson], [main])
# FIXME: Replace `main' with a function in `-lz':
#AC_CHECK_LIB([z], [main])

#AC_CHECK_LIB([apr], [apr_socket_recv])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h sys/timeb.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memset pow strcasecmp strchr strdup strstr strtol])

AC_CONFIG_FILES([Makefile
                 common/Makefile
                 db/Makefile
                 server/Makefile])
AC_OUTPUT
