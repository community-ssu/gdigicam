#
# Makefile.am for GDigicam
#

SUBDIRS=gst-camerabin

MAINTAINERCLEANFILES = Makefile.in

CLEANFILES = \
	*.gcno \
	*.gcda

if BUILD_TESTS

  TESTS = check_test

  TESTS_ENVIRONMENT = \
	CK_FORK=yes   \
	CK_VERBOSITY=verbose

  check_test_LDADD = \
	$(top_builddir)/src/libgdigicam-@GDIGICAM_API_VERSION@.la \
	$(GDIGICAM_LIBS)	\
	$(GDIGICAM_MAEMO_LIBS)	\
	$(GCONF_LIBS)		\
	$(X11_LIBS)		\
	$(GTK_LIBS)

  check_test_LDFLAGS = \
	-module -avoid-version	\
	$(COV_LDFLAGS)

  check_test_CFLAGS = \
	-I$(top_srcdir)		\
	-I$(top_srcdir)/src	\
	$(GDIGICAM_CFLAGS)	\
	$(GDIGICAM_MAEMO_CFLAGS)\
	$(GCONF_CFLAGS)		\
	$(X11_CFLAGS)		\
	$(GTK_CFLAGS)		\
	$(DEBUG_CFLAGS)		\
	$(OPT_CFLAGS)		\
	$(LOG_CFLAGS)		\
	$(COV_CFLAGS)

else
  TESTS =
endif

noinst_PROGRAMS	= $(TESTS)

check_test_SOURCES = \
	check_test.c  \
	check-utils.c \
	check-gdigicam-manager.c

