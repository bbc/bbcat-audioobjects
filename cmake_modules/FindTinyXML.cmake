# Try to find the TinyXML library
# TinyXML_FOUND - system has TinyXML
# TinyXML_INCLUDE_DIRS - TinyXML include directory
# TinyXML_LIBRARY_DIRS - TinyXML library directory
# TinyXML_LIBRARIES - TinyXML libraries

# Copyright (C) 2012  iCub Facility, Istituto Italiano di Tecnologia
# Author: Daniele E. Domenichelli <daniele.domenichelli@iit.it>
#
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

if(WIN32)
	set(TinyXML_INSTALL_DIR "${WIN32_LIB_DESTINATION}/TinyXML")
	set(TinyXML_INCLUDE_DIRS "${TinyXML_INSTALL_DIR}" CACHE PATH "TinyXML include directory" FORCE)
	set(TinyXML_LIBRARY_DIRS "${TinyXML_INSTALL_DIR}/x64/Release" CACHE PATH "TinyXML library directory" FORCE)
	set(TinyXML_LIBRARIES "${TinyXML_LIBRARY_DIRS}/tinyxmlSTL.lib" CACHE STRING "TinyXML libraries" FORCE)	
else(WIN32)
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        if(TinyXML_FIND_VERSION)
            if(TinyXML_FIND_VERSION_EXACT)
                pkg_check_modules(PC_TINYXML QUIET tinyxml=${TinyXML_FIND_VERSION})
            else(TinyXML_FIND_VERSION_EXACT)
                pkg_check_modules(PC_TINYXML QUIET tinyxml>=${TinyXML_FIND_VERSION})
            endif(TinyXML_FIND_VERSION_EXACT)
        else(TinyXML_FIND_VERSION)
            pkg_check_modules(PC_TINYXML QUIET tinyxml)
        endif(TinyXML_FIND_VERSION)

		set(TinyXML_INCLUDE_DIRS ${PC_TINYXML_INCLUDE_DIRS} CACHE PATH "TinyXML include directory" FORCE)
		set(TinyXML_LIBRARY_DIRS ${PC_TINYXML_LIBRARY_DIRS} CACHE PATH "TinyXML library directory" FORCE)
		set(TinyXML_LIBRARIES ${PC_TINYXML_LIBRARIES} CACHE STRING "TinyXML libraries" FORCE)
    endif(PKG_CONFIG_FOUND)
endif(WIN32)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TinyXML
                                  DEFAULT_MSG
                                  TinyXML_LIBRARIES
)

set(TinyXML_FOUND ${TINYXML_FOUND})

mark_as_advanced(TinyXML_INCLUDE_DIRS TinyXML_LIBRARY_DIRS TinyXML_LIBRARIES)
