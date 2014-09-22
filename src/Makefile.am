pkgincludedir = $(includedir)/aplibs-audioobjects-@APLIBS_AUDIOOBJECTS_MAJORMINOR@/aplibs-audioobjects

lib_LTLIBRARIES = libap-audioobjects-@APLIBS_AUDIOOBJECTS_MAJORMINOR@.la

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_LDFLAGS =	\
	-no-undefined													\
	-version-info $(APLIBS_AUDIOOBJECTS_LIBVERSION)					\
     $(APLIBS_AUDIOOBJECTS_LDFLAGS)

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_LIBADD =	\
	$(APLIBS_DSP_LIBS)											\
	$(APLIBS_RENDER_LIBS)										\
	$(TINYXML_LIBS)

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_CPPFLAGS =	\
	$(APLIBS_AUDIOOBJECTS_CFLAGS)									\
	$(APLIBS_DSP_CFLAGS)											\
	$(APLIBS_RENDER_CFLAGS)											\
	$(TINYXML_CFLAGS)

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_SOURCES =	\
	ADMAudioFileSamples.cpp											\
	ADMData.cpp														\
	ADMFileWriteRenderer.cpp										\
	ADMObjects.cpp													\
	ADMRIFFFile.cpp													\
	FilePositionGenerator.cpp										\
	PlaybackEngine.cpp												\
	Playlist.cpp													\
	RIFFChunk.cpp													\
	RIFFChunks.cpp													\
	RIFFFile.cpp													\
	SoundFileAttributes.cpp											\
	TinyXMLADMData.cpp												\
	crc32.cpp														\
	mt19937ar.c

pkginclude_HEADERS =							\
	ADMAudioFileSamples.h						\
	ADMData.h									\
	ADMFileWriteRenderer.h						\
	ADMObjects.h								\
	ADMRIFFFile.h								\
	FilePositionGenerator.h						\
	PlaybackEngine.h							\
	Playlist.h									\
	RIFFChunk.h									\
	RIFFChunk_Definitions.h						\
	RIFFChunks.h								\
	RIFFFile.h									\
	SoundFileAttributes.h						\
	SoundFileWithPosition.h						\
	TinyXMLADMData.h							\
	crc32.h										\
	mt19937ar.h

noinst_HEADERS =

nodist_pkginclude_HEADERS = aplibs-audioobjects-stdint.h

DISTCLEANFILES = aplibs-audioobjects-stdint.h
