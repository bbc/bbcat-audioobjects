
AUTOMAKE_OPTIONS = foreign

include doxygen.am

SUBDIRS = src

audioobjects_DATA  = share/licences.txt share/standarddefinitions.xml

audioobjectsdir = $(prefix)/share/$(PACKAGE)

EXTRA_DIST = README COPYING autogen.sh $(audioobjects_DATA)

pkgconfig_DATA = bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR).pc

bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR).pc: bbcat-audioobjects.pc
	cp bbcat-audioobjects.pc bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR).pc

bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc: bbcat-audioobjects-uninstalled.pc
	cp bbcat-audioobjects-uninstalled.pc bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES = bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc gitbranch.txt

CLEANFILES = bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR).pc bbcat-audioobjects-$(BBCAT_AUDIOOBJECTS_MAJORMINOR)-uninstalled.pc src/SelfRegisteringObjects.h src/version.h src/version.cpp gitbranch.txt .branch

ACLOCAL_FLAGS = -I m4

.PHONY: gitbranch.txt

gitbranch.txt:
	@-git rev-parse --abbrev-ref HEAD >.branch && (diff gitbranch.txt .branch >/dev/null || cp .branch gitbranch.txt)
	@if [ ! -e "gitbranch.txt" ] ; then touch "gitbranch.txt" ; fi
	@echo "Branch: `cat gitbranch.txt`"
