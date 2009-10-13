# gdigicam package version number, (as distinct from shared library version)
# An odd micro number indicates in-progress developmen
# An even micro number indicates a released version.
m4_define(gdigicam_major_version, 0)
m4_define(gdigicam_minor_version, 2)
m4_define(gdigicam_micro_version, 40)

m4_define([gdigicam_version],
          [gdigicam_major_version.gdigicam_minor_version.gdigicam_micro_version])

m4_define([gdigicam_api_version],
          [gdigicam_major_version.gdigicam_minor_version])

# increase the interface age for each release; if the API changes, set to 0
m4_define([gdigicam_interface_age], [0])
m4_define([gdigicam_binary_age],
          [m4_eval(100 * gdigicam_minor_version + gdigicam_micro_version)])

AC_PREREQ(2.53)

AC_INIT([gdigicam], [gdigicam_version],
        [alexander.bokovoy@nokia.com])
dnl         [https://bugs.maemo.org/enter_bug.cgi?product=gdigicam])
AC_CONFIG_SRCDIR([src/gdigicam.h])
AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([1.7 foreign])

GDIGICAM_MAJOR_VERSION=gdigicam_major_version
GDIGICAM_MINOR_VERSION=gdigicam_minor_version
GDIGICAM_MICRO_VERSION=gdigicam_micro_version
GDIGICAM_VERSION=gdigicam_version
GDIGICAM_API_VERSION=gdigicam_api_version
GDIGICAM_MAJORMINOR=gdigicam_api_version
AC_SUBST(GDIGICAM_MAJOR_VERSION)
AC_SUBST(GDIGICAM_MINOR_VERSION)
AC_SUBST(GDIGICAM_MICRO_VERSION)
AC_SUBST(GDIGICAM_VERSION)
AC_SUBST(GDIGICAM_API_VERSION)
AC_SUBST(GDIGICAM_MAJORMINOR)

m4_define([lt_current],
          [m4_eval(100 * gdigicam_minor_version + gdigicam_micro_version - gdigicam_interface_age)])
m4_define([lt_revision], [gdigicam_interface_age])
m4_define([lt_age], [m4_eval(gdigicam_binary_age - gdigicam_interface_age)])

GDIGICAM_LT_CURRENT=lt_current
GDIGICAM_LT_REV=lt_revision
GDIGICAM_LT_AGE=lt_age
GDIGICAM_LT_VERSION="$GDIGICAM_LT_CURRENT:$GDIGICAM_LT_REV:$GDIGICAM_LT_AGE"
GDIGICAM_LT_LDFLAGS="-version-info $GDIGICAM_LT_VERSION"

AC_SUBST(GDIGICAM_LT_VERSION)
AC_SUBST(GDIGICAM_LT_LDFLAGS)

dnl = parameters ====================================================

if test "x${prefix}" = "xNONE";
  then gdigicam_prefix=${ac_default_prefix};
  else gdigicam_prefix=${prefix}
fi

dnl If the user didn't specify the $sysconfdir on the command line,
dnl let it be /etc, not /usr/local/etc or /usr/etc
if [[ x$sysconfdir = 'x${prefix}/etc' ]]; then
   sysconfdir=/etc
fi

if test "x{$sysconfdir}" = "xNONE";
  then confdir=${gdigicam_prefix}/etc;
  else confdir=$sysconfdir;
fi

GREETING_STRING="$PACKAGE_NAME: Open Source GDigicam"
AC_DEFINE_UNQUOTED(GREETING,"$GREETING_STRING",[The greeting string])

AC_ARG_WITH(config-dir,
  [  --with-config-dir=DIR     Directory for config files],
CONFIG_DIR="${withval}", CONFIG_DIR="${confdir}/gdigicam")

AC_SUBST(CONFIG_DIR)

AC_DEFINE_UNQUOTED(CONFIG_DIR,"$CONFIG_DIR",[GDigicam configuration])

dnl =  determine over what platform to build ===============================

AC_MSG_NOTICE([Determining what platform to build])
PKG_CHECK_MODULES(LIBOSSO,libosso,[with_platform=maemo],true)

AC_ARG_WITH(platform,
[--with-platform=gnome		platform to build for [gnome]],
[case "${with_platform}" in
       gnome) GDIGICAM_PLATFORM=gnome;;
       maemo) GDIGICAM_PLATFORM=maemo;;
       *)     GDIGICAM_PLATFORM=${with_platform}
esac], [GDIGICAM_PLATFORM=gnome
               with_platform=gnome])
AC_SUBST(GDIGICAM_PLATFORM)

if test "x$with_platform" = "xmaemo"; then

   PKG_CHECK_MODULES(HILDON, hildon-1 >= 0.9.9, hildon1=yes, hildon1=no)
   if test "$hildon1" == "yes"; then   # the 'new' hildon
       PKG_CHECK_MODULES(GDIGICAM_MAEMO, hildon-1 libosso)

   else	                               # the old hildon
       PKG_CHECK_MODULES(GDIGICAM_MAEMO, hildon-libs >= 0.12.0 libosso)
       AC_DEFINE_UNQUOTED(GDIGICAM_HILDON_VERSION_0, 1, ["The Hildon version we support."])
   fi

  AC_DEFINE_UNQUOTED(GDIGICAM_PLATFORM_MAEMO, 1, ["Whether GDigicam is being built for the Maemo platform."])

  # don't check format errors, because it breaks with logical id's
  CFLAGS="$CFLAGS -Wno-format-extra-args"
else
   AC_DEFINE_UNQUOTED(GDIGICAM_PLATFORM_GNOME, 1, ["Whether GDigicam is being built for the GNOME platform."])
fi

dnl ========================================================================

# Checks for programs.
AC_PROG_CC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap strcasecmp strdup])

dnl = Specify some additional warnings =====================================

dnl if test "x$GCC" = "xyes"; then
dnl         GCC_FLAGS="-g -Wall -Wmissing-prototypes -Wmissing-declarations"
dnl fi

dnl = Check for Performance measurement =========================================

AC_ARG_ENABLE(performance,[--enable-performance performance macros will be enabled],,enable_performance=no)

if test "x$enable_performance" = "xyes"; then
   AC_DEFINE_UNQUOTED(GDIGICAM_PERFORMANCE, 1, ["Whether GDigicam is being built with performance analysis macros."])
   AC_SUBST(GDIGICAM_PERFORMANCE)
fi

dnl = Additinoal CFLAGS =========================================

dnl ====================================
dnl = Additional CFLAGS
dnl ====================================

# Option to enable debugging
AC_ARG_ENABLE(debug,
              [AC_HELP_STRING([  --enable-debug],[Debugging (default=no)])],
              [with_debug=yes], [with_debug=no])

# Option to enable optimized code
AC_ARG_ENABLE(opt,
              [AC_HELP_STRING([  --enable-opt],[Optimizing (default=no)])],
              [with_opt=yes], [with_opt=no])

AC_ARG_ENABLE(log,
              [AC_HELP_STRING([  --enable-log],[Logging (default=no)])],
              [with_log=yes], [with_log=no])

# Option to enable code coverage
AC_ARG_ENABLE(coverage,
              [AC_HELP_STRING([  --enable-coverage],[Compile with coverage (default=no)])],
              [enable_coverage=yes], [enable_coverage=no])

if test x$enable_coverage = xyes; then
  if test "x$with_debug" == "xyes" ; then
    DEBUG_CFLAGS="$WARN_CFLAGS -ggdb -DDEBUG -Werror -DGST_USE_UNSTABLE_API -Wall -Wunreachable-code"
  else
    DEBUG_CFLAGS="$WARN_CFLAGS -ggdb -DDEBUG -Wall -Wunreachable-code"
  fi
  AC_PATH_PROG(LCOV, [lcov], [lcov])
  AC_PATH_PROG(GENHTML, [genhtml], [genhtml])
  if test "x$LCOV" = x; then
    echo You need to install lcov to get actual reports!
    echo See http://ltp.sf.net/coverage/lcov.php
  fi
  if test "x$SBOX_USE_CCACHE" == xyes; then
    AC_MSG_ERROR([Please set SBOX_USE_CCACHE=no to use coverage.])
  fi
  COV_CFLAGS="-fprofile-arcs -ftest-coverage"
  COV_LDFLAGS="-lgcov"
  AC_SUBST(COV_CFLAGS)
  AC_SUBST(COV_LDFLAGS)

  with_opt=no
else
  if test "x$with_debug" == "xyes" ; then
    DEBUG_CFLAGS="$WARN_CFLAGS -ggdb -DDEBUG -Werror -DGST_USE_UNSTABLE_API -Wall -Wunreachable-code"

    with_opt=no
  else
    DEBUG_CFLAGS=""
  fi
fi
AC_SUBST(DEBUG_CFLAGS)
dnl AC_SUBST(CFLAGS)

if test "x$with_opt" == "xyes" ; then
dnl   OPT_CFLAGS=" -O2 -DG_DISABLE_CAST_CHECKS"
  OPT_CFLAGS=" -O2 -DG_DISABLE_CAST_CHECKS -DG_DISABLE_ASSERT"
else
  OPT_CFLAGS=" -O0"
fi
AC_SUBST(OPT_CFLAGS)

if test "x$with_log" == "xyes" ; then
  LOG_CFLAGS=" -DOSSOLOG_COMPILE"
fi
AC_SUBST(LOG_CFLAGS)

AM_CONDITIONAL(ENABLE_COVERAGE,
               [test "x$enable_coverage" != xno && test -n "$LCOV" && test -n "$GENHTML"])

dnl ========================================================================

gdigicamlib=libgdigicam-$GDIGICAM_API_VERSION.la
gdigicamgstcamerabinlib=libgdigicam-gst-camerabin-$GDIGICAM_API_VERSION.la

dnl = Check for glib =======================================================

GLIB_REQUIRED=2.2
AC_SUBST(GLIB_REQUIRED)

PKG_CHECK_MODULES(GLIB, glib-2.0 >= $GLIB_REQUIRED)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

dnl = Check for gobject ====================================================

PKG_CHECK_MODULES(GOBJECT, gobject-2.0)
AC_SUBST(GOBJECT_CFLAGS)
AC_SUBST(GOBJECT_LIBS)

dnl dnl = Check for GCONF =======================================================

GCONF_REQUIRED=2.2
AC_SUBST(GCONF_REQUIRED)

PKG_CHECK_MODULES(GCONF, [gconf-2.0 >= $GCONF_REQUIRED])
AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

dnl = Check for GdkPixbuf ==================================================

GDKPIXBUF_REQUIRED=2.12.12
AC_SUBST(GDKPIXBUF_REQUIRED)

PKG_CHECK_MODULES(GDKPIXBUF, gdk-pixbuf-2.0 >= $GDKPIXBUF_REQUIRED)

AC_SUBST(GDKPIXBUF_LIBS)
AC_SUBST(GDKPIXBUF_CFLAGS)

dnl = Check for Gtk+, which is optional and used in examples ================

HAVE_GTK=NO
PKG_CHECK_MODULES(GTK2, gtk+-2.0 >= 2.2.0, HAVE_GTK_22=yes, HAVE_GTK_22=no)
if test "x$HAVE_GTK_22" = "xyes"; then
  HAVE_GTK=yes
  GTK_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
  AC_SUBST(GTK_VERSION)
  GTK_PREFIX=`$PKG_CONFIG --variable=prefix gdk-pixbuf-2.0`
  AC_SUBST(GTK_BASE_DIR)
  GDK_PIXBUF_LIBDIR=`$PKG_CONFIG --variable=libdir gdk-pixbuf-2.0`
  GDK_PIXBUF_PREFIXDIR=`$PKG_CONFIG --variable=prefix gdk-pixbuf-2.0`
  AC_SUBST(GTK_BASE_DIR)
else
  PKG_CHECK_MODULES(GTK2, gtk+-2.0, HAVE_GTK_20=yes, HAVE_GTK_20=no)
fi
if test "x$HAVE_GTK_20" = "xyes"; then
  HAVE_GTK=yes
fi
GTK_CFLAGS=$GTK2_CFLAGS
GTK_LIBS=$GTK2_LIBS
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(HAVE_GTK)
AM_CONDITIONAL(HAVE_GTK, test "x$HAVE_GTK" = "xyes")

dnl = Check for GStreamer ==================================================

GST_REQUIRED=0.10.0
GST_MAJORMINOR=0.10
AC_SUBST(GST_REQUIRED)
AC_SUBST(GST_MAJORMINOR)

dnl Gstreamer in Maemo
PKG_CHECK_MODULES(GST, \
  gstreamer-$GST_MAJORMINOR >= $GST_REQUIRED
  gstreamer-plugins-base-$GST_MAJORMINOR  >= $GST_REQUIRED,
  HAVE_GST=yes,HAVE_GST=no)

dnl Check again in Ubuntu/Debian
if test "x$HAVE_GST" = "xno"; then
  PKG_CHECK_MODULES(GST, \
    gstreamer-$GST_MAJORMINOR >= $GST_REQUIRED
    gstreamer-base-$GST_MAJORMINOR >= $GST_REQUIRED,
    HAVE_GST=yes,HAVE_GST=no)
fi

dnl Give error and exit if we don't have GStreamer
if test "x$HAVE_GST" = "xno"; then
  AC_MSG_ERROR(you need gstreamer development packages installed !)
else
  GST_LIBS="$GST_LIBS -lgstinterfaces-$GST_MAJORMINOR"
fi

dnl GST_LIBS="$GST_LIBS -lgstinterfaces-$GST_MAJORMINOR -lgstvideo-$GST_MAJORMINOR -lgstaudio-$GST_MAJORMINOR"

dnl make GST_CFLAGS and GST_LIBS available
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_LIBS)

dnl Required only to execute unit tests and examples
dnl if test "x$enable_tests" = "xyes"; then
dnl    AC_CHECK_LIB(gstxvimagesink, gst_video_sink_center_rect, [], AC_MSG_ERROR(No package 'gstreamer0.10-x' found))
dnl    AC_CHECK_LIB(gstximagesink, gst_ximage_buffer_get_type, [], AC_MSG_ERROR(No package 'gstreamer0.10-x' found))
dnl    AC_CHECK_LIB(gstxvimagesink, gst_video_sink_center_rect, [], AC_MSG_ERROR(No package 'gstreamer0.10-plugins-base' found))

dnl fi

dnl = Check for GStreamer Bad ===========================================

dnl Gstreamer bad in Maemo
PKG_CHECK_MODULES(GSTBAD, \
  gstreamer-plugins-bad-0.10  >= $GST_REQUIRED,
  HAVE_GSTBAD=yes,HAVE_GSTBAD=no)

dnl Give error and exit if we don't have GStreamer Bad
dnl FIXME: In the future we should enable conditional compilation
if test "xHAVE_GSTBAD" = "xno"; then
  AC_MSG_ERROR(you need gstreamer plugins bad development package installed !)
fi

AC_SUBST(GSTBAD_CFLAGS)
AC_SUBST(GSTBAD_LIBS)

dnl Gstreamer video in Maemo ==============================================

GST_VIDEO_REQUIRED=0.10

PKG_CHECK_MODULES(GST_VIDEO, [gstreamer-video-0.10  >= $GST_VIDEO_REQUIRED])

AC_SUBST(GST_VIDEO_CFLAGS)
AC_SUBST(GST_VIDEO_LIBS)

dnl = GTK Doc check ========================================================

GTK_DOC_CHECK([1.6])

if test "x$enable_gtk_doc" = "xyes"; then
  AC_DEFINE(BUILD_REFERENCE, [1], [We want to build reference])
  BUILD_GTK_DOC=yes
else
  BUILD_GTK_DOC=no
fi
AM_CONDITIONAL(BUILD_GTK_DOC, test "x$BUILD_GTK_DOC" = "xyes")

dnl = REBUILD of generated source code files ===============================

# Disable rebuild of glib-mkenum -generated source code:
AC_ARG_ENABLE(rebuilds, [  --disable-rebuilds      disable all source autogeneration rules],,enable_rebuilds=yes)

# define a MAINT-like variable REBUILD which is set if Perl
# is found, so autogenerated sources can be rebuilt
AC_CHECK_PROGS(PERL, perl5 perl)

REBUILD=\#
if test "x$enable_rebuilds" = "xyes" && \
     test -n "$PERL" && \
     $PERL -e 'exit !($] >= 5.002)' > /dev/null 2>&1 ; then
  REBUILD=
fi
AC_SUBST(REBUILD)

dnl = Check and X11 support (c unit test) =================================

BUILD_TESTS=no
if test "x$enable_tests" = "xyes"; then
   PKG_CHECK_MODULES(CHECK, check , HAVE_CHECK="yes", HAVE_CHECK="no")
   if test "x$HAVE_CHECK" = "xyes"; then
      HAVE_X11=no
      PKG_CHECK_MODULES(X11, x11 >= 1.1.1, HAVE_X11=yes, HAVE_X11=no)
      if test "x$HAVE_X11" = "xyes"; then
         if test "x$HAVE_GTK" = "xyes"; then
            BUILD_TESTS=yes
         else
            AC_MSG_ERROR(you need Gtk+ development packages installed to build unit tests!)
         fi

      AC_SUBST(X11_CFLAGS)
      AC_SUBST(X11_LIBS)
      else
         AC_MSG_ERROR(you need X11 development packages installed to build unit tests!)
      fi

   AC_SUBST(CHECK_CFLAGS)
   AC_SUBST(CHECK_LIBS)
   else
      AC_MSG_ERROR(you need check development packages installed to build unit tests!)
   fi
fi
AM_CONDITIONAL(BUILD_TESTS, test "x$BUILD_TESTS" = "xyes")


dnl = Examples support ====================================================

BUILD_EXAMPLES=no
if test "x$enable_examples" = "xyes"; then
   if test "x$HAVE_GTK" = "xyes"; then
      BUILD_EXAMPLES=yes
   else
      AC_MSG_ERROR(you need Gtk+ development packages installed to build examples!)
   fi
fi
AM_CONDITIONAL(BUILD_EXAMPLES, test "x$BUILD_EXAMPLES" = "xyes")

dnl = Config file support =================================================

AC_ARG_ENABLE(config-file,[--enable-config-file lets user tweak config file],,enable_config_file=no)
if test "x$enable_config_file" = "xyes"; then
   AC_DEFINE([USE_CONFIG_FILE],[1],[Defined if config file enabled])
   AC_SUBST(USE_CONFIG_FILE)
fi

dnl ========================================================================

AC_SUBST(GCC_FLAGS)

GDIGICAM_CFLAGS="$GLIB_CFLAGS $GOBJECT_CFLAGS $GST_CFLAGS $GDKPIXBUF_CFLAGS $CHECK_CFLAGS"
GDIGICAM_LIBS="$GLIB_LIBS $GOBJECT_LIBS $GST_LIBS $GDKPIXBUF_LIBS $CHECK_LIBS"

AC_SUBST(GDIGICAM_CFLAGS)
AC_SUBST(GDIGICAM_LIBS)

AC_OUTPUT([
        Makefile
        debian/libgdigicam0.install
        debian/libgdigicam-dev.install
        debian/libgdigicam-doc.install
        debian/libgdigicam-gst-camerabin0.install
        debian/libgdigicam-gst-camerabin-dev.install
        debian/libgdigicam-gst-camerabin-doc.install
        doc/Makefile
        doc/reference/Makefile
        doc/reference/version.xml
        doc/reference/gst-camerabin/Makefile
        doc/reference/gst-camerabin/version.xml
        examples/Makefile
        ext/Makefile
        ext/gst-camerabin/Makefile
        ext/gst-camerabin/gdigicam-gst-camerabin.pc
	ext/gst-camerabin/data/Makefile
        src/Makefile
        src/gdigicam-version.h
        tests/Makefile
        tests/gst-camerabin/Makefile
        gdigicam.pc
])

dnl ========================================================================

echo ""
echo "                  GDigicam $VERSION"
echo "                 ==================="
echo ""
dnl echo "     source code location:   ${srcdir}"
echo "                   prefix:   ${prefix}"
dnl echo "                 data dir:   ${DATA_DIR}"
dnl echo "               config dir:   ${CONFIG_DIR}"
echo "                 compiler:   ${CC}"
echo "                 platform:   ${with_platform}"
echo ""
echo "           Target library:   ${gdigicamlib}"
echo "            Ext libraries:   ${gdigicamgstcamerabinlib}"
echo "  Performance measurement:   ${enable_performance}"
echo "                Debugging:   ${with_debug}"
echo "             Optimization:   ${with_opt}"
echo "                  Logging:   ${with_log}"
echo "            Documentation:   ${enable_gtk_doc}"
echo "             Unit testing:   ${BUILD_TESTS}"
echo "                 Coverage:   ${enable_coverage}"
echo "                 Examples:   ${BUILD_EXAMPLES}"
echo "       Configuration file:   ${enable_config_file}"
echo ""
