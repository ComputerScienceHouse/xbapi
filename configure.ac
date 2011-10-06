#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([xbapi], [0.1], [russ@eatnumber1.com])
AC_CONFIG_SRCDIR([include/xbapi.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CC_C99
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDBOOL
AC_HEADER_ASSERT

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MEMMOVE
AC_CHECK_FUNCS([getaddrinfo freeaddrinfo inet_ntop gai_strerror])
AC_CHECK_FUNCS([htonl htons ntohl ntohs])

# Checks for talloc library
PKG_CHECK_MODULES([TALLOC], [talloc])
PKG_CHECK_MODULES([CUNIT], [cunit])

AC_CONFIG_FILES([Makefile xbapi.pc])
AC_OUTPUT
