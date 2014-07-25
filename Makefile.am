
AUTOMAKE_OPTIONS = foreign

include doxygen.am

SUBDIRS = src

EXTRA_DIST = README COPYING autogen.sh

pkgconfig_DATA = aplibs-audioobjects-$(APLIBS_AUDIOOBJECTS_MAJORMINOR).pc

aplibs-audioobjects-$(APLIBS_AUDIOOBJECTS_MAJORMINOR).pc: aplibs-audioobjects.pc
	cp aplibs-audioobjects.pc aplibs-audioobjects-$(APLIBS_AUDIOOBJECTS_MAJORMINOR).pc

aplibs-audioobjects-$(APLIBS_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc: aplibs-audioobjects-uninstalled.pc
	cp aplibs-audioobjects-uninstalled.pc aplibs-audioobjects-$(APLIBS_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES = aplibs-audioobjects-$(APLIBS_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc

CLEANFILES = aplibs-audioobjects-$(APLIBS_AUDIOOBJECTS_MAJORMINOR).pc aplibs-audioobjects-$(APLIBS_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc

ACLOCAL_FLAGS = -I m4
