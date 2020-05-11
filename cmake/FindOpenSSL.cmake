# https://github.com/warmcat/libwebsockets/blob/master/scripts/FindLibWebSockets.cmake
# This module tries to find libWebsockets library and include files
#
# OPENSSL_INCLUDE_DIR, path where to find libwebsockets.h
# OPENSSL_LIBRARIES_DIR, path where to find libwebsockets.so
# OPENSSL_LIBRARIES, the library to link against
# OPENSSL_FOUND, If false, do not try to use libWebSockets
# 
# This currently works probably only for Linux
#mkdir -p build
#cd build
#cmake -DCMAKE_CXX_FLAGS="-lrt" ..
#cmake -DBUILD_SHARED_LIBS=1 -DCMAKE_CXX_FLAGS="-lrt" ..
#make

find_package(PkgConfig)
pkg_check_modules(OPENSSL_PKG openssl)

if (NOT OPENSSL_PKG_INCLUDE_DIRS OR NOT OPENSSL_PKG_LIBRARY_DIRS)
  find_path(
    OPENSSL_INCLUDE_DIRS 
    NAMES openssl.h crypto.h PATH_SUFFIXES openssl
    PATHS 
    /usr/include/
    /usr/include/openssl
    /usr/local/include/
  )    
  find_library(
    OPENSSL_LIBRARY
    NAMES ssl 
    PATHS
    /usr/lib
    /usr/local/lib
  )
  
  find_library(
    CRYPTO_LIBRARY
    NAMES crypto
    PATHS
    /usr/lib
    /usr/local/lib
  )  
  set(OPENSSL_LIBRARIES 
    ${OPENSSL_LIBRARY}
    ${CRYPTO_LIBRARY}
  )       
else()
  find_path(
    OPENSSL_INCLUDE_DIRS 
    NAMES openssl.h crypto.h PATH_SUFFIXES openssl
    PATHS ${OPENSSL_PKG_INCLUDE_DIRS}
  )
  
  find_library(
    OPENSSL_LIBRARY
    NAMES ssl
    PATHS ${OPENSSL_PKG_LIBRARY_DIRS}
  )  
  
  find_library(
    CRYPTO_LIBRARY
    NAMES crypto
    PATHS ${OPENSSL_PKG_LIBRARY_DIRS}
  ) 
  
  set(OPENSSL_LIBRARIES 
    ${OPENSSL_LIBRARY}
    ${CRYPTO_LIBRARY}
  )   
endif()

if (OPENSSL_INCLUDE_DIRS AND OPENSSL_LIBRARIES)
  MESSAGE(STATUS "Found openssl: ${OPENSSL_INCLUDE_DIRS},${OPENSSL_LIBRARIES}")
else()
  MESSAGE(SENDERR "Could NOT find libssl")
endif()

MARK_AS_ADVANCED(
  OPENSSL_INCLUDE_DIRS
  OPENSSL_LIBRARIES
)
