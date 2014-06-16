pkgincludedir = $(includedir)/aplibs-audioobjects-@APLIBS_AUDIOOBJECTS_MAJORMINOR@/aplibs-audioobjects

lib_LTLIBRARIES = libap-audioobjects-@APLIBS_AUDIOOBJECTS_MAJORMINOR@.la

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_LDFLAGS =	\
	-no-undefined													\
	-version-info $(APLIBS_AUDIOOBJECTS_LIBVERSION)					\
     $(APLIBS_AUDIOOBJECTS_LDFLAGS)

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_LIBADD =	\
	$(APLIBSDSP_LIBS)											\
	$(TINYXML_LIBS)

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_CPPFLAGS =	\
	$(APLIBS_AUDIOOBJECTS_CFLAGS)									\
	$(APLIBSDSP_CFLAGS)												\
	$(TINYXML_CFLAGS)

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_SOURCES =	\
	ADMAudioFileSamples.cpp											\
	ADMData.cpp														\
	ADMObjects.cpp													\
	ADMRIFFFile.cpp													\
	crc32.cpp														\
	mt19937ar.c														\
	RIFFChunk.cpp													\
	RIFFChunks.cpp													\
	RIFFFile.cpp													\
	SoundFileAttributes.cpp											\
	TinyXMLADMData.cpp

pkginclude_HEADERS =							\
	ADMAudioFileSamples.h						\
	ADMData.h									\
	ADMObjects.h								\
	ADMRIFFFile.h								\
	crc32.h										\
	mt19937ar.h									\
	RIFFChunk_Definitions.h						\
	RIFFChunk.h									\
	RIFFChunks.h								\
	RIFFFile.h									\
	SoundFileAttributes.h						\
	SoundFileWithPosition.h						\
	TinyXMLADMData.h

noinst_HEADERS =

nodist_pkginclude_HEADERS = aplibs-audioobjects-stdint.h

DISTCLEANFILES = aplibs-audioobjects-stdint.h
