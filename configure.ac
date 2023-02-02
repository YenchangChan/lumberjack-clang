#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([lumberjack], [1.0.0], [chenyanchang1990@163.com])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

AC_CANONICAL_HOST



# Checks for programs.
AC_LANG(C)
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_LIBTOOL
AC_PROG_CXX

# OS/ARCH specific hacks
case $host in
    *ibm-aix6*)
    CFLAGS="$CFLAGS -qcpluscmt -O3"
    LIBS="$LIBS -lm -lpthread"
    AC_DEFINE([AIX], 1, [Define to 1 if you are using aix.])
    ;;
    *ibm-aix7*)
    CFLAGS="$CFLAGS -qcpluscmt -O3"
    LIBS="$LIBS -lm -lpthread"
    AC_DEFINE([AIX], 1, [Define to 1 if you are using aix.])
    ;;
    *linux*)
	CXXFLAGS="$CXXFLAGS -std=c++11"
    LIBS="$LIBS -lpthread"
    ;;
esac

AC_ARG_WITH(openssl,
            [ --with-openssl=DIR   directory to search openssl static library ],
            [ LIBSSL="$withval" ],
            [ LIBSSL= ],
            )
AM_CONDITIONAL([HAVE_SSL_H], [test x$LIBSSL != x])

AC_ARG_WITH(zlib,
            [ --with-zlib=DIR   directory to search zlib static library ],
            [ LIBZLIB="$withval"  ],
            [ LIBZLIB=""  ]
            )
AM_CONDITIONAL([HAVE_ZLIB_H], [test x$LIBZLIB != x])


AC_SUBST(LIBSSL)
AC_SUBST(LIBZLIB)
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T


# Checks for library functions.
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero gettimeofday select socket strcasecmp strchr strdup])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
