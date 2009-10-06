#
# Makefile.am for GDigicam
#

SUBDIRS 				= src							\
					  ext	 						\
					  examples 						\
					  doc							\
					  tests
# 					  pkgconfig	 					\
# 					  po

EXTRA_DIST 				= gdigicam.pc.in
					  debian/changelog 					\
					  debian/compat 					\
					  debian/control 					\
					  debian/copyright	 				\
					  debian/docs	 					\
					  debian/libgdigicam-dev.install.in			\
					  debian/libgdigicam-doc.install.in	 		\
					  debian/libgdigicam-gst-camerabin-dev.install.in	\
					  debian/libgdigicam-gst-camerabin0.install.in 		\
					  debian/libgdigicam-gst-camerabin-doc.install.in	\
					  debian/libgdigicam-tests.install	 		\
					  debian/libgdigicam0.install.in	 		\
					  debian/rules						\
					  debian/run_tests

pkgconfig_DATA 				= gdigicam-@GDIGICAM_API_VERSION@.pc

pkgconfigdir   				=  $(libdir)/pkgconfig

CLEANFILES 				= \
					  gdigicam-@GDIGICAM_API_VERSION@.pc 			\
					  *.log 						\
					  *.gcno						\
					  *.gcda

DISTCLEANFILES 				= \
					  gdigicam.pc						\
					  debian/libgdigicam-dev.install			\
					  debian/libgdigicam-doc.install	 		\
					  debian/libgdigicam-gst-camerabin-dev.install		\
					  debian/libgdigicam-gst-camerabin0.install 		\
					  debian/libgdigicam-gst-camerabin-doc.install		\
					  debian/libgdigicam0.install

DISTCHECK_CONFIGURE_FLAGS 		= --enable-gtk-doc

MAINTAINERCLEANFILES			= Makefile.in aclocal.m4 configure	 		\
			  		  config.h.in compile 					\
			  		  ltmain.sh depcomp missing 		    		\
			  		  install-sh config.sub 		    		\
			  		  config.guess intltool-extract.in 	    		\
			  		  intltool-merge.in intltool-update.in      		\
			  		  mkinstalldirs build-stamp configure-stamp

gdigicam-@GDIGICAM_API_VERSION@.pc: gdigicam.pc
	@cp -f gdigicam.pc gdigicam-@GDIGICAM_API_VERSION@.pc

if ENABLE_COVERAGE
LCOV_DATA_DIR				= lcov-data
LCOV_DATA_FILE				= lcov.info

distclean-local:
	-rm -rf $(LCOV_DATA_DIR)

lcov-zero-counters:
	$(LCOV) -z -d .

lcov:
	-mkdir -p $(LCOV_DATA_DIR)
	$(LCOV) -c -d . -o $(LCOV_DATA_DIR)/$(LCOV_DATA_FILE)
	$(GENHTML) -s $(LCOV_DATA_DIR)/$(LCOV_DATA_FILE) -o $(LCOV_DATA_DIR)
	@echo
	@echo "Please, have a look on ./$(LCOV_DATA_DIR)/index.html for coverage statistics"
	@echo

endif

# deb: dist
# 	-mkdir debian-build
# 	cd debian-build && \
# 	tar zxvf ../$(PACKAGE)-$(VERSION).tar.gz && \
# 	cd $(PACKAGE)-$(VERSION) && dpkg-buildpackage -rfakeroot
