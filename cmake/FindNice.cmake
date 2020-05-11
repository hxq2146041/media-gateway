# https://github.com/warmcat/libwebsockets/blob/master/scripts/FindLibWebSockets.cmake
# This module tries to find libWebsockets library and include files
#
# NICE_INCLUDE_DIR, path where to find libwebsockets.h
# NICE_LIBRARIES_DIR, path where to find libwebsockets.so
# NICE_LIBRARIES, the library to link against
# NICE_FOUND, If false, do not try to use libWebSockets
#
# This currently works probably only for Linux

find_package(PkgConfig)
pkg_check_modules(NICE_PKG nice)

if (NOT NICE_PKG_INCLUDE_DIRS OR NOT NICE_PKG_LIBRARY_DIRS)
  find_path(
    NICE_INCLUDE_DIRS 
    NAMES agent.h PATH_SUFFIXES nice
    PATHS 
    /usr/include/
    /usr/include/nice
    /usr/local/include/
    /usr/local/include/nice
  )    
  find_library(
    NICE_LIBRARIES
    NAMES nice
    PATHS
    /usr/lib
    /usr/local/lib
  )
       
else()
  find_path(
    NICE_INCLUDE_DIRS 
    NAMES agent.h PATH_SUFFIXES nice
    PATHS ${NICE_PKG_INCLUDE_DIRS}
  )
  
  find_library(
    NICE_LIBRARIES
    NAMES nice
    PATHS ${NICE_PKG_LIBRARY_DIRS}
  )   
endif()

if (NICE_INCLUDE_DIRS AND NICE_LIBRARIES)
  MESSAGE(STATUS "Found nice: ${NICE_INCLUDE_DIRS},${NICE_LIBRARIES}")
else()
  MESSAGE(SENDERR "Could NOT find libnice")
endif()

MARK_AS_ADVANCED(
  NICE_INCLUDE_DIRS
  NICE_LIBRARIES
)
