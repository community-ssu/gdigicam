#! /bin/sh
gtkdocize --docdir doc/reference || exit 1
gtkdocize --docdir doc/reference/gst-camerabin || exit 1
autoreconf -v --install || exit 1
