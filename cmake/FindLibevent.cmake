# find LibEvent
# an event notification library (http://libevent.org/)
#
# Usage:
# LIBEVENT_INCLUDE_DIRS, where to find LibEvent headers
# LIBEVENT_LIBRARIES, LibEvent libraries
# LIBEVENT_ENABLE, If false, do not try to use libevent

#set(LIBEVENT_ROOT CACHE PATH "Root directory of libevent installation")
#set(LibEvent_EXTRA_PREFIXES /usr/local /opt/local "$ENV{HOME}" ${LIBEVENT_ROOT})

#foreach(prefix ${LibEvent_EXTRA_PREFIXES})
#  list(APPEND LibEvent_INCLUDE_PATHS "${prefix}/include")
#  list(APPEND LibEvent_LIBRARIES_PATHS "${prefix}/lib")
#endforeach()

find_package(PkgConfig)
pkg_check_modules(LIBEVENT_PKG libevent)

if(LIBEVENT_PKG_FOUND)
  find_path(LIBEVENT_INCLUDE_DIRS
    NAMES evhttp.h event.h
    PATHS ${LIBEVENT_PKG_INCLUDE_DIRS}
  )  
  find_library(LIBEVENT_LIBRARIES
    NAMES event event_core event_extra libevent
    PATHS ${LIBEVENT_PKG_LIBRARY_DIRS}
  )  
else()
  find_path(LIBEVENT_INCLUDE_DIRS
    NAMES evhttp.h event.h
    PATHS 
    /usr/include
    /usr/local/include
  ) 
  find_library(LIBEVENT_LIBRARIES 
    NAMES event event_core event_extra libevent 
    PATHS 
    /usr/lib
    /usr/local/lib
  )
endif()

if (LIBEVENT_LIBRARIES AND LIBEVENT_INCLUDE_DIRS)
  set(LIBEVENT_ENABLE 1)
  set(LIBEVENT_LIBRARIES ${LIBEVENT_LIBRARIES})
else ()
  set(LIBEVENT_ENABLE 0)
endif ()

if (LIBEVENT_ENABLE)
  if (NOT Libevent_FIND_QUIETLY)
    message(STATUS "Found libevent: ${LIBEVENT_INCLUDE_DIRS},${LIBEVENT_LIBRARIES}")
  endif ()
else ()
  if (LibEvent_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find libevent.")
  endif ()
  message(STATUS "libevent NOT found.")
endif ()

mark_as_advanced(
  LIBEVENT_LIBRARIES
  LIBEVENT_INCLUDE_DIRS
)
