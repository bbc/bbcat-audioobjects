pkgincludedir = $(includedir)/aplibs-audioobjects-@APLIBS_AUDIOOBJECTS_MAJORMINOR@/aplibs-audioobjects

lib_LTLIBRARIES = libap-audioobjects-@APLIBS_AUDIOOBJECTS_MAJORMINOR@.la

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_LDFLAGS =	\
	-no-undefined													\
	-version-info $(APLIBS_AUDIOOBJECTS_LIBVERSION)					\
     $(APLIBS_AUDIOOBJECTS_LDFLAGS)

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_LIBADD =	\
	$(APLIBS_DSP_LIBS)

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_CPPFLAGS =	\
	$(APLIBS_AUDIOOBJECTS_CFLAGS)									\
	$(APLIBS_DSP_CFLAGS)

libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_SOURCES =	\
	ADMAudioFileSamples.cpp											\
	ADMData.cpp														\
	ADMObjects.cpp													\
	ADMRIFFFile.cpp													\
	Playlist.cpp													\
	RIFFChunk.cpp													\
	RIFFChunks.cpp													\
	RIFFFile.cpp													\
	SoundFileAttributes.cpp

pkginclude_HEADERS =							\
	ADMAudioFileSamples.h						\
	ADMData.h									\
	ADMObjects.h								\
	ADMRIFFFile.h								\
	Playlist.h									\
	RIFFChunk.h									\
	RIFFChunk_Definitions.h						\
	RIFFChunks.h								\
	RIFFFile.h									\
	SoundFileAttributes.h						\
	SoundFileWithPosition.h

if ENABLE_GPL
libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_CPPFLAGS += $(TINYXML_CFLAGS)
libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_LIBADD   += $(TINYXML_LIBS)
libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_SOURCES 	+= TinyXMLADMData.cpp
pkginclude_HEADERS												+= TinyXMLADMData.h
endif

if ENABLE_RENDERING
libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_CPPFLAGS += $(APLIBS_RENDER_CFLAGS)
libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_LIBADD   += $(APLIBS_RENDER_LIBS)
libap_audioobjects_@APLIBS_AUDIOOBJECTS_MAJORMINOR@_la_SOURCES 	+=	\
	ADMFileWriteRenderer.cpp										\
	FilePositionGenerator.cpp										\
	PlaybackEngine.cpp

pkginclude_HEADERS +=							\
	ADMFileWriteRenderer.h						\
	FilePositionGenerator.h						\
	PlaybackEngine.h
endif

noinst_HEADERS =

nodist_pkginclude_HEADERS = aplibs-audioobjects-stdint.h

DISTCLEANFILES = aplibs-audioobjects-stdint.h
