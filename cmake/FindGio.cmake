
message(STATUS "PKG_CONFIG_LIBDIR:${PKG_CONFIG_LIBDIR}")

find_package(PkgConfig)

pkg_check_modules(GIO_PKG gio-2.0)

if(NOT GIO_PKG_FOUND)
  MESSAGE("CMAKE not FOUND GIO_PKG_FOUND")
  find_path(GIO_INCLUDE_DIRS NAMES gio.h PATH_SUFFIXES gio-2.0
    PATHS
    /usr/include/gio-2.0
    /usr/include
    /usr/local/include
  )
  
  find_path(GIO_CONFIG_INCLUDE_DIRS NAMES glibconfig.h
    PATHS 
    /usr/include/gio-2.0/include
    /usr/include
    /usr/local/include
  )

  find_library(GIO_LIBRARIES NAMES gio-2.0
    PATHS
    /usr/lib
    /usr/local/lib
  )
else()
  find_path(GIO_INCLUDE_DIRS NAMES gio.h PATH_SUFFIXES gio-2.0
    PATHS ${GIO_PKG_INCLUDE_DIRS}
  )
  
  find_path(GIO_CONFIG_INCLUDE_DIRS NAMES glibconfig.h PATH_SUFFIXES gio-2.0
    PATHS ${GIO_PKG_INCLUDE_DIRS}
  )
  
  find_library(GIO_LIBRARIES NAMES gio-2.0
    PATHS ${GIO_PKG_LIBRARY_DIRS}
  )    
endif()

if(GIO_INCLUDE_DIRS AND GIO_LIBRARIES AND GIO_CONFIG_INCLUDE_DIRS)
  message(STATUS "Found gio-2.0: ${GIO_INCLUDE_DIRS}, ${GIO_LIBRARIES}")
else(NOT GIO_INCLUDE_DIRS OR GIO_LIBRARIES)
  message(SENDERR "gio-2.0 not found.")
endif()

#mark_as_advanced(GIO_CONFIG_INCLUDE_DIRS GIO_INCLUDE_DIRS GIO_LIBRARIES)
mark_as_advanced(GIO_INCLUDE_DIRS GIO_LIBRARIES)

