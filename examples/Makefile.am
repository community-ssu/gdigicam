#
# Makefile.am for GDigicam
#

MAINTAINERCLEANFILES 			= Makefile.in

CLEANFILES				= *.gcno *.gcda

if BUILD_EXAMPLES
noinst_PROGRAMS				= gdigicam_manager_test_app

# Main example
gdigicam_manager_test_app_LDADD		= $(top_builddir)/src/libgdigicam-@GDIGICAM_API_VERSION@.la		\
					  $(GDIGICAM_LIBS)							\
					  $(GDIGICAM_MAEMO_LIBS)						\
					  $(GTK_LIBS)
gdigicam_manager_test_app_CFLAGS	= -I$(top_srcdir)							\
					  -I$(top_srcdir)/src							\
					  $(GDIGICAM_CFLAGS)							\
					  $(GDIGICAM_MAEMO_CFLAGS)						\
					  $(GTK_CFLAGS)								\
					  $(DEBUG_CFLAGS)							\
					  $(OPT_CFLAGS)								\
					  $(LOG_CFLAGS)								\
					  $(COV_CFLAGS)

gdigicam_manager_test_app_SOURCES	= gdigicam_manager_test_app.c

endif

EXTRA_DIST = \
	README													\
	gdigicam_manager_test_app.c
