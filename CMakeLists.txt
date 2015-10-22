# Arbitrarily set minimum version.
# This specific version is used to avoid complaints about the 
# cygwin platform not defining WIN32 anymore.
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.12.2)

# The name of our top-level project.
PROJECT( bbcat-audioobjects C CXX )

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_RELEASE 1)
set(PROJECT_VERSION_BUILD 1)

#include all the parts that are consistent across all libraries
include(CMakeLists-bbcat.txt)

find_package(bbcat-control REQUIRED)
include_directories(${BBCAT_CONTROL_INCLUDE_DIRS})
add_definitions(${BBCAT_CONTROL_FLAGS})

find_package(TinyXML REQUIRED)
include_directories(${TinyXML_INCLUDE_DIRS})

set(GLOBAL_INCLUDE_DIRS
	${BBCAT_CONTROL_INCLUDE_DIRS}
	${INSTALL_INCLUDE_DIR}
	${TINYXML_INCLUDE_DIRS})

set(GLOBAL_FLAGS
	${BBCAT_CONTROL_FLAGS})

# setup BBCAT_AUDIOOBJECTS_DATA_FILES definition
set(GLOBAL_FLAGS
	${GLOBAL_FLAGS}
	"-D${UPPERCASE_PROJECT_NAME}_DATA_FILES=\"${INSTALL_SHARE_DIR}\""
	"-DTIXML_USE_STL")

ADD_DEFINITIONS(${GLOBAL_FLAGS})

set(_extra_libs
	${_extra_libs}
	${TinyXML_LIBRARIES})

################################################################################
# sources are contained in the src/ directory
ADD_SUBDIRECTORY( src )

################################################################################
# install files for 'share'
install(DIRECTORY "share/"
		DESTINATION "${INSTALL_SHARE_DIR}"
		PATTERN "share/*")
