# https://github.com/warmcat/libwebsockets/blob/master/scripts/FindLibWebSockets.cmake
# This module tries to find libWebsockets library and include files
#
# LIBWEBSOCKETS_INCLUDE_DIR, path where to find libwebsockets.h
# LIBWEBSOCKETS_LIBRARIES_DIR, path where to find libwebsockets.so
# LIBWEBSOCKETS_LIBRARIES, the library to link against
# LIBWEBSOCKETS_FOUND, If false, do not try to use libWebSockets
#
# This currently works probably only for Linux
#cmake -DCMAKE_INSTALL_PREFIX=/usr -DLWS_OPENSSL_INCLUDE_DIRS=/usr/include/ -DLWS_OPENSSL_LIBRARIES="/usr/lib/libssl.a;/usr/lib/libcrypto.a" -DLWS_WITH_BORINGSSL=ON -DLWS_WITHOUT_TEST_SERVER=ON -DLWS_WITHOUT_TESTAPPS=ON -DLWS_WITHOUT_TEST_PING=ON -DLWS_WITHOUT_TEST_CLIENT=ON -DCMAKE_SYSTEM_NAME="Linux" -DCMAKE_C_FLAGS="-fPIC" ..

#cmake -DCMAKE_INSTALL_PREFIX=/usr -DLWS_OPENSSL_INCLUDE_DIRS=/usr/include -DLWS_OPENSSL_LIBRARIES="/usr/lib/libssl.so;/usr/lib/libcrypto.so" -DLWS_WITH_BORINGSSL=ON -DLWS_WITHOUT_TEST_SERVER=ON -DLWS_WITHOUT_TESTAPPS=ON -DLWS_WITHOUT_TEST_PING=ON -DLWS_WITHOUT_TEST_CLIENT=ON -DCMAKE_C_FLAGS="-fPIC" ..


find_package(PkgConfig)
pkg_check_modules(LIBWEBSOCKETS_PKG libwebsockets)

if (NOT LIBWEBSOCKETS_PKG_INCLUDE_DIRS OR NOT LIBWEBSOCKETS_PKG_LIBRARY_DIRS)
  find_path(
    LIBWEBSOCKETS_INCLUDE_DIRS 
    NAMES libwebsockets.h
    PATHS 
    /usr/include
		/usr/local/include
  )    
  find_library(
    LIBWEBSOCKETS_LIBRARIES
    NAMES websockets
    PATHS
    /usr/lib
    /usr/local/lib
  )       
else()
  find_path(
    LIBWEBSOCKETS_INCLUDE_DIRS 
    NAMES libwebsockets.h
    PATHS ${LIBWEBSOCKETS_PKG_INCLUDE_DIRS}
  )
  
  find_library(
    LIBWEBSOCKETS_LIBRARIES
    NAMES websockets
    PATHS ${LIBWEBSOCKETS_PKG_LIBRARY_DIRS}
  )   
endif()


if (LIBWEBSOCKETS_INCLUDE_DIRS AND LIBWEBSOCKETS_LIBRARIES)
  MESSAGE(STATUS "Found Libwebsockets: ${LIBWEBSOCKETS_INCLUDE_DIRS},${LIBWEBSOCKETS_LIBRARIES}")
else()
  MESSAGE(SENDERR "Could NOT find Libwebsockets")
endif()

MARK_AS_ADVANCED(
  LIBWEBSOCKETS_INCLUDE_DIRS
  LIBWEBSOCKETS_LIBRARIES
)
