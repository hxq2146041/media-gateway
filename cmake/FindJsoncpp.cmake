
# Find the jsoncpp includes and library
# 
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH 
# 
# This module defines
#  JSONCPP_INCLUDE_DIRS, where to find header, etc.
#  JSONCPP_LIBRARIES, the libraries needed to use jsoncpp.
#  JSONCPP_FOUND, If false, do not try to use jsoncpp.
#  JSONCPP_INCLUDE_PREFIX, include prefix for jsoncpp

# only look in default directories


find_package(PkgConfig)

pkg_check_modules(JSONCPP_PKG jsoncpp)
  
if(NOT JSONCPP_PKG_FOUND)
  find_path(
    JSONCPP_INCLUDE_DIRS 
    NAMES jsoncpp/json/json.h json/json.h
    PATHS ${JSONCPP_PKG_INCLUDE_DIRS}
  )
  
  find_library(
    JSONCPP_LIBRARIES
    NAMES jsoncpp
    PATHS ${JSONCPP_PKG_LIBRARY_DIRS}
  )  
else()
  find_path(
    JSONCPP_INCLUDE_DIRS 
    NAMES jsoncpp/json/json.h json/json.h
    PATHS 
    /usr/lib/include
    /usr/local/lib/include
  )
  
  find_library(
    JSONCPP_LIBRARIES
    NAMES jsoncpp
    PATHS 
    /usr/lib
    /usr/local/lib
  )
endif()

if(JSONCPP_INCLUDE_DIRS AND JSONCPP_LIBRARIES)
  message(STATUS "Found jsoncpp: ${JSONCPP_INCLUDE_DIRS}, ${JSONCPP_LIBRARIES}")
  set(JSONCPP_INCLUDE_DIR ${JSONCPP_INCLUDE_DIRS})
else()
  message(SENDERR "jsoncpp not found.")
endif()

# debug library on windows
# same naming convention as in qt (appending debug library with d)
# boost is using the same "hack" as us with "optimized" and "debug"
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  find_library(
	  JSONCPP_LIBRARIES_DEBUG
	  NAMES jsoncppd
	  DOC "jsoncpp debug library"
  )
	set(JSONCPP_LIBRARIES optimized ${JSONCPP_LIBRARIES} debug ${JSONCPP_LIBRARIES_DEBUG})
endif()

# find JSONCPP_INCLUDE_PREFIX
find_path(
	JSONCPP_INCLUDE_PREFIX
	NAMES json.h
	PATH_SUFFIXES jsoncpp/json json
)

if (${JSONCPP_INCLUDE_PREFIX} MATCHES "jsoncpp")
	set(JSONCPP_INCLUDE_PREFIX "jsoncpp/json")
else()
	set(JSONCPP_INCLUDE_PREFIX "json")
endif()

# handle the QUIETLY and REQUIRED arguments and set JSONCPP_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(jsoncpp DEFAULT_MSG
	 JSONCPP_LIBRARIES)
	
mark_as_advanced (JSONCPP_INCLUDE_DIRS JSONCPP_LIBRARIES)

