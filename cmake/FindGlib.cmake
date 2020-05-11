
message(STATUS "PKG_CONFIG_LIBDIR:${PKG_CONFIG_LIBDIR}")

find_package(PkgConfig)

pkg_check_modules(GLIB_PKG glib-2.0)

if(NOT GLIB_PKG_FOUND)
  MESSAGE("CMAKE not FOUND GLIB_PKG_FOUND")
  find_path(GLIB_INCLUDE_DIRS NAMES glib.h PATH_SUFFIXES glib-2.0
    PATHS
    /usr/include/glib-2.0
    /usr/include
    /usr/local/include
  )
  
  find_path(GLIB_CONFIG_INCLUDE_DIRS NAMES glibconfig.h
    PATHS 
    /usr/include/glib-2.0/include
    /usr/include
    /usr/local/include
  )

  find_library(GLIB_LIBRARIES NAMES glib-2.0
    PATHS
    /usr/lib
    /usr/local/lib
  )
else()
  find_path(GLIB_INCLUDE_DIRS NAMES glib.h PATH_SUFFIXES glib-2.0
    PATHS ${GLIB_PKG_INCLUDE_DIRS}
  )
  
  find_path(GLIB_CONFIG_INCLUDE_DIRS NAMES glibconfig.h PATH_SUFFIXES glib-2.0
    PATHS ${GLIB_PKG_INCLUDE_DIRS}
  )
  
  find_library(GLIB_LIBRARIES NAMES glib-2.0
    PATHS ${GLIB_PKG_LIBRARY_DIRS}
  )    
endif()

if(GLIB_INCLUDE_DIRS AND GLIB_LIBRARIES AND GLIB_CONFIG_INCLUDE_DIRS)
  message(STATUS "Found glib-2.0: ${GLIB_INCLUDE_DIRS}, ${GLIB_LIBRARIES}")
else(NOT GLIB_INCLUDE_DIRS OR GLIB_LIBRARIES)
  message(SENDERR "glib-2.0 not found.")
endif()

#mark_as_advanced(GLIB_CONFIG_INCLUDE_DIRS GLIB_INCLUDE_DIRS GLIB_LIBRARIES)
mark_as_advanced(GLIB_CONFIG_INCLUDE_DIRS GLIB_INCLUDE_DIRS GLIB_LIBRARIES)

