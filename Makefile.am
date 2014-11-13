
AUTOMAKE_OPTIONS = foreign

include doxygen.am

SUBDIRS = src

EXTRA_DIST = README COPYING autogen.sh

pkgconfig_DATA = bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR).pc

bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR).pc: bbcat-audioobjects.pc
	cp bbcat-audioobjects.pc bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR).pc

bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc: bbcat-audioobjects-uninstalled.pc
	cp bbcat-audioobjects-uninstalled.pc bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES = bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc

CLEANFILES = bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR).pc bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc src/SelfRegisteringObjects.h

ACLOCAL_FLAGS = -I m4
