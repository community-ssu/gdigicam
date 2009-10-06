#
# Makefile.am for GDigicam
#

SUBDIRS=data

INCLUDES = \
	-I$(top_srcdir)		\
	-I${top_srcdir}/src	\
	-DG_LOG_DOMAIN=\"gdigicam-gstcamerabin-@GDIGICAM_API_VERSION@\" 	\
	@GCC_FLAGS@		\
	@GDIGICAM_CFLAGS@	\
	@GSTBAD_CFLAGS@		\
	$(GST_VIDEO_CFLAGS)     \
	$(DEBUG_CFLAGS)		\
	$(OPT_CFLAGS)		\
	$(LOG_CFLAGS)		\
	$(COV_CFLAGS)

lib_LTLIBRARIES	= \
	libgdigicam-gst-camerabin-@GDIGICAM_API_VERSION@.la

libgdigicam_gst_camerabin_@GDIGICAM_API_VERSION@_la_LIBADD = \
	$(top_srcdir)/src/libgdigicam-@GDIGICAM_API_VERSION@.la \
	@GDIGICAM_LIBS@		\
	@GSTBAD_LIBS@           \
	$(GST_VIDEO_LIBS)

libgdigicam_gst_camerabin_@GDIGICAM_API_VERSION@_la_LDFLAGS = \
	@GDIGICAM_LT_LDFLAGS@ \
	$(COV_LDFLAGS)

libgdigicam_gst_camerabin_@GDIGICAM_API_VERSION@_la_CFLAGS = \
	@GDIGICAM_CFLAGS@

libgdigicam_gst_camerabin_@GDIGICAM_API_VERSION@_la_SOURCES = \
	gdigicam-camerabin.c

libgdigicam_gst_camerabin_@GDIGICAM_API_VERSION@_includedir = \
	$(includedir)/$(PACKAGE)-@GDIGICAM_API_VERSION@/$(PACKAGE)/gst-camerabin

# gdigicamheadersdir = $(includedir)/gdigicam-@GDIGICAM_API_VERSION@/gdigicam

libgdigicam_gst_camerabin_@GDIGICAM_API_VERSION@_public_headers = \
	gdigicam-camerabin.h

libgdigicam_gst_camerabin_@GDIGICAM_API_VERSION@_include_HEADERS= \
	$(libgdigicam_gst_camerabin_@GDIGICAM_API_VERSION@_public_headers)

gdigicam-gst-camerabin-@GDIGICAM_API_VERSION@.pc: gdigicam-gst-camerabin.pc
	@cp -f gdigicam-gst-camerabin.pc gdigicam-gst-camerabin-@GDIGICAM_API_VERSION@.pc


CLEANFILES	= gdigicam-gst-camerabin-@GDIGICAM_API_VERSION@.pc *.gcno *.gcda
DISTCLEANFILES	= gdigicam-gst-camerabin.pc
MAINTAINERCLEANFILES = Makefile.in


EXTRA_DIST	= gdigicam-gst-camerabin.pc.in
pkgconfig_DATA	= gdigicam-gst-camerabin-@GDIGICAM_API_VERSION@.pc
pkgconfigdir  	=  $(libdir)/pkgconfig

