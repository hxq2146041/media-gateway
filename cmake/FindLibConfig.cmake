# Find the CUnit includes and library
#
# This module defines
# LIBCONFIG_INCLUDE_DIR, where to find cppunit include files, etc.
# LIBCONFIG_LIBRARIES, the libraries to link against to use CppUnit.
# LIBCONFIG_STATIC_LIBRARIY_PATH
# LIBCONFIG_FOUND, If false, do not try to use CppUnit.

# also defined, but not for general use are
# LIBCONFIG_LIBRARIES, where to find the CUnit library.

#MESSAGE("Searching for libconfig library")

find_package(PkgConfig)

pkg_check_modules(LIBCONFIG_PKG libconfig)

if(LIBCONFIG_PKG_FOUND)
  find_path(
    LIBCONFIG_INCLUDE_DIRS 
    NAMES libconfig.h
    PATHS ${LIBCONFIG_PKG_INCLUDE_DIRS}
  )
  
	

  find_library(
    LIBCONFIG_LIBRARIES
    NAMES config
    PATHS ${LIBCONFIG_PKG_LIBRARY_DIRS}
  ) 
  
	

else()
  find_path(
    LIBCONFIG_INCLUDE_DIRS 
    NAMES libconfig.h
    PATHS 
    /usr/local/include
    /usr/include
  )
  
  find_library(
    LIBCONFIG_LIBRARIES
    NAMES libconfig
    PATHS 
    /usr/lib/
    /usr/local/lib
  )
endif()

if (LIBCONFIG_INCLUDE_DIRS AND LIBCONFIG_LIBRARIES)
  message(STATUS "Found libconfig : ${LIBCONFIG_INCLUDE_DIRS},${LIBCONFIG_LIBRARIES}" )
else()
  message(SENDERROR "Could NOT find LibConfig")
endif()

MARK_AS_ADVANCED(LIBCONFIG_INCLUDE_DIRS LIBCONFIG_LIBRARIES)
