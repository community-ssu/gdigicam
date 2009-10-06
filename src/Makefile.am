#
# Makefile.am for GDigicam
#

INCLUDES = \
	-I$(top_srcdir)	\
	-DG_LOG_DOMAIN=\"gdigicam-@GDIGICAM_API_VERSION@\" \
	@GCC_FLAGS@ 		\
	@GDIGICAM_CFLAGS@	\
	$(DEBUG_CFLAGS)		\
	$(OPT_CFLAGS)		\
	$(LOG_CFLAGS)		\
	$(COV_CFLAGS)


libgdigicam_built_sources = \
	gdigicam-marshal.c

libgdigicam_built_headers = \
	gdigicam-marshal.h

lib_LTLIBRARIES = \
	libgdigicam-@GDIGICAM_API_VERSION@.la

libgdigicam_@GDIGICAM_API_VERSION@_la_LIBADD = \
	@GDIGICAM_LIBS@

libgdigicam_@GDIGICAM_API_VERSION@_la_LDFLAGS = \
	@GDIGICAM_LT_LDFLAGS@ \
	$(COV_LDFLAGS)

libgdigicam_@GDIGICAM_API_VERSION@_la_CFLAGS = \
	@GDIGICAM_CFLAGS@

libgdigicam_@GDIGICAM_API_VERSION@_la_SOURCES = \
	$(libgdigicam_built_sources)	\
	gdigicam-error.c		\
	gdigicam-manager.c		\
	gdigicam-util.c

libgdigicam_@GDIGICAM_API_VERSION@_includedir = \
	$(includedir)/$(PACKAGE)-@GDIGICAM_API_VERSION@/$(PACKAGE)

# gdigicamheadersdir = $(includedir)/gdigicam-@GDIGICAM_API_VERSION@/gdigicam

libgdigicam_@GDIGICAM_API_VERSION@_public_headers = \
	$(libgdigicam_built_headers)	\
	gdigicam-error.h		\
	gdigicam-debug.h		\
	gdigicam-manager.h		\
	gdigicam-util.h			\
	gdigicam.h 			\
	gdigicam-version.h

libgdigicam_@GDIGICAM_API_VERSION@_include_HEADERS = \
	$(libgdigicam_@GDIGICAM_API_VERSION@_public_headers)

noinst_HEADERS	= gdigicam-manager-private.h

gdigicam-marshal.h: gdigicam-marshal.list
	glib-genmarshal --prefix=gdigicam_marshal --header gdigicam-marshal.list > gdigicam-marshal.h

gdigicam-marshal.c:gdigicam-marshal.list gdigicam-marshal.h
	echo '#include "gdigicam-marshal.h"' >gdigicam-marshal.c
	glib-genmarshal --prefix=gdigicam_marshal --body gdigicam-marshal.list >> gdigicam-marshal.c


EXTRA_DIST = gdigicam-marshal.list

MAINTAINERCLEANFILES = \
	Makefile.in \
	gdigicam-marshal.h \
	gdigicam-marshal.c

CLEANFILES = \
	*.gcno \
	*.gcda


