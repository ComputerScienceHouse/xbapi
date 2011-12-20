ACLOCAL_AMFLAGS = -I m4

CFLAGS += -Wall -Werror -Wextra

lib_LTLIBRARIES = libxbapi.la
libxbapi_la_SOURCES = src/xbapi.c src/packets.c
libxbapi_la_CFLAGS = -I$(srcdir)/include -fvisibility=hidden $(TALLOC_CFLAGS)
libxbapi_la_LDFLAGS = -no-undefined -version-info 0:0:0 $(TALLOC_LIBS)

libxbapi_la_includedir = $(includedir)
libxbapi_la_include_HEADERS = include/xbapi.h

check_PROGRAMS = cunit

cunit_SOURCES = test/src/cunit.c test/src/test_xbapi.c src/packets.c
cunit_CFLAGS = -I$(srcdir)/include -I$(srcdir)/src -I$(srcdir)/test/include $(TALLOC_CFLAGS) $(CUNIT_CFLAGS)
cunit_LDFLAGS = $(TALLOC_LIBS) $(CUNIT_LIBS)

TESTS = $(check_PROGRAMS)

noinst_HEADERS = include/_xbapi.h include/_packets.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xbapi.pc
