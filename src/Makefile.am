pkgincludedir = $(includedir)/bbcat-audioobjects-@BBCAT_AUDIOOBJECTS_MAJORMINOR@/bbcat-audioobjects

lib_LTLIBRARIES = libap-audioobjects-@BBCAT_AUDIOOBJECTS_MAJORMINOR@.la

libap_audioobjects_@BBCAT_AUDIOOBJECTS_MAJORMINOR@_la_LDFLAGS =	\
	-no-undefined												\
	-version-info $(BBCAT_AUDIOOBJECTS_LIBVERSION)				\
     $(BBCAT_AUDIOOBJECTS_LDFLAGS)

libap_audioobjects_@BBCAT_AUDIOOBJECTS_MAJORMINOR@_la_LIBADD =	\
	$(BBCAT_BASE_LIBS)											\
	$(BBCAT_DSP_LIBS)

libap_audioobjects_@BBCAT_AUDIOOBJECTS_MAJORMINOR@_la_CPPFLAGS =	\
	$(BBCAT_AUDIOOBJECTS_CFLAGS)									\
	$(BBCAT_BASE_CFLAGS)											\
	$(BBCAT_DSP_CFLAGS)

libap_audioobjects_@BBCAT_AUDIOOBJECTS_MAJORMINOR@_la_SOURCES =	\
	ADMAudioFileSamples.cpp										\
	ADMData.cpp													\
	ADMObjects.cpp												\
	ADMRIFFFile.cpp												\
	Playlist.cpp												\
	RIFFChunk.cpp												\
	RIFFChunks.cpp												\
	RIFFFile.cpp												\
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
libap_audioobjects_@BBCAT_AUDIOOBJECTS_MAJORMINOR@_la_CPPFLAGS += $(TINYXML_CFLAGS)
libap_audioobjects_@BBCAT_AUDIOOBJECTS_MAJORMINOR@_la_LIBADD   += $(TINYXML_LIBS)
libap_audioobjects_@BBCAT_AUDIOOBJECTS_MAJORMINOR@_la_SOURCES  += TinyXMLADMData.cpp
pkginclude_HEADERS											   += TinyXMLADMData.h
endif

if ENABLE_RENDERING
libap_audioobjects_@BBCAT_AUDIOOBJECTS_MAJORMINOR@_la_CPPFLAGS += $(BBCAT_RENDER_CFLAGS)
libap_audioobjects_@BBCAT_AUDIOOBJECTS_MAJORMINOR@_la_LIBADD   += $(BBCAT_RENDER_LIBS)
libap_audioobjects_@BBCAT_AUDIOOBJECTS_MAJORMINOR@_la_SOURCES +=	\
	ADMFileWriteRenderer.cpp										\
	FilePositionGenerator.cpp										\
	PlaybackEngine.cpp

pkginclude_HEADERS +=							\
	ADMFileWriteRenderer.h						\
	FilePositionGenerator.h						\
	PlaybackEngine.h
endif

noinst_HEADERS =
