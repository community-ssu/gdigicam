#!/bin/sh
./autogen.sh
#./configure --enable-maintainer-mode "$@"
./configure --enable-maintainer-mode --enable-debug --enable-tests --enable-examples --enable-gtk-doc --enable-config-file "$@"
