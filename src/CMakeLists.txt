#sources
set(_sources
	ADMAudioFileSamples.cpp
	ADMData.cpp
	ADMObjects.cpp
	ADMRIFFFile.cpp
	ADMXMLGenerator.cpp
	Playlist.cpp
	RIFFChunk.cpp
	RIFFChunks.cpp
	RIFFFile.cpp
	SoundFileAttributes.cpp
	SoundObjectFile.cpp
	TinyXMLADMData.cpp
	XMLADMData.cpp
	XMLValue.cpp
	${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)

# public headers
set(_headers
	ADMAudioFileSamples.h
	ADMData.h
	ADMObjects.h
	ADMRIFFFile.h
	ADMXMLGenerator.h
	Playlist.h
	RIFFChunk.h
	RIFFChunk_Definitions.h
	RIFFChunks.h
	RIFFFile.h
	SoundFileAttributes.h
	SoundObjectFile.h
	TinyXMLADMData.h
	XMLADMData.h
	XMLValue.h
	${CMAKE_CURRENT_BINARY_DIR}/version.h
)

#include all the parts that are consistent across all libraries
include(CMakeLists-src.txt)

TARGET_LINK_LIBRARIES(bbcat-audioobjects bbcat-control bbcat-dsp bbcat-base)
