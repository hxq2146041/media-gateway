
cmake_minimum_required(VERSION 3.5)

project(media-gateway)

include(CheckFunctionExists)

# Abseil requires C++11
set(CMAKE_CXX_STANDARD 11)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

string(REPLACE "\\" "/" CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

if (CMAKE_INSTALL_PREFIX)
	set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
endif()

if(NOT LIBCONFIG_LIBRARIES OR NOT LIBCONFIG_INCLUDE_DIRS)
  find_package(LibConfig REQUIRED)
endif()

if(NOT GLIB_LIBRARIES OR NOT GLIB_INCLUDE_DIRS)
  find_package (Glib 2.0 REQUIRED REQUIRED)
endif()

if(NOT LIBWEBSOCKETS_INCLUDE_DIRS OR NOT LIBWEBSOCKETS_LIBRARIES)
  find_package(LibWebSockets REQUIRED)
endif()

if(NOT OPENSSL_INCLUDE_DIRS OR NOT OPENSSL_LIBRARIES)
  find_package(OpenSSL REQUIRED)
endif()

if(NOT NICE_INCLUDE_DIRS OR NOT NICE_LIBRARIES)
  find_package(Nice REQUIRED)
endif()

#find_package (Jansson REQUIRED)

if (NOT DEFINED WINDOWS AND NOT DEFINED ANDROID AND NOT DEFINED IOS AND NOT DEFINED MAC AND NOT DEFINED LINUX)
  set(LINUX 1)
endif()

if (WINDOWS)
  set(WEBRTC_COMPILE_DEFINITIONS 
    -DNDEBUG
    -DWEBRTC_WIN
    -DABSL_ALLOCATOR_NOTHROW=1 
  )  
  message(STATUS "windows compile...")   
  set(RTC_ENABLE_LIBEVENT 0)
endif ()

if (ANDROID)
  set(WEBRTC_ANDROID 1)
  set(WEBRTC_POSIX 1)
  set(WEBRTC_LINUX 1)  
  set(WEBRTC_COMPILE_DEFINITIONS 
    -DNDEBUG
    -DWEBRTC_POSIX
    -DWEBRTC_LINUX
    -DABSL_ALLOCATOR_NOTHROW=1 
  )
  message(STATUS "android compile...")  
endif ()

if (IOS)
  add_definitions(-DWEBRTC_IOS)
  set(WEBRTC_IOS 1)
  message(STATUS "ios compile...")
endif ()

if (MAC)
  set(WEBRTC_MAC 1)
  add_definitions(-DWEBRTC_MAC)
endif ()

if (LINUX)  
  set(WEBRTC_POSIX 1)
  set(WEBRTC_LINUX 1)
  set(WEBRTC_COMPILE_DEFINITIONS 
    -DNDEBUG
    -DDCHECK_ALWAYS_ON
    -DWEBRTC_POSIX
    -DWEBRTC_LINUX
    -DABSL_ALLOCATOR_NOTHROW=1 
  )
     
  set(WEBRTC_C_FLAGS "\
    -fno-strict-aliasing \
    --param=ssp-buffer-size=4 \
    -fstack-protector \
    -funwind-tables \
    -fPIC \
    -pipe \
    -pthread \
    -m64 \
    -march=x86-64 \
    -Wno-builtin-macro-redefined \
    -D__DATE__= -D__TIME__= -D__TIMESTAMP__= \
    -Wall \
    -Wno-unused-local-typedefs \
    -Wno-maybe-uninitialized \
    -Wno-deprecated-declarations \
    -Wno-comments \
    -Wno-packed-not-aligned \
    -Wno-missing-field-initializers \
    -Wno-unused-parameter \
    -O2 \
    -fno-ident \
    -fdata-sections \
    -ffunction-sections \
    -fno-omit-frame-pointer \
    -g0 \
    -fvisibility=hidden \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-missing-field-initializers \
  ")
  
  SET(WEBRTC_CXX_FLAGS "\
    -std=gnu++11 \
    -Wno-narrowing \
    -Wno-class-memaccess \
    -fno-exceptions \
    -fno-rtti \
    -fvisibility-inlines-hidden \
    -Wnon-virtual-dtor \
    -Woverloaded-virtual \
    -Wno-deprecated-declarations \
    -Wno-deprecated-declarations \
    -Wno-undef \
  ")

  SET(WEBRTC_LINK_OPTIONS "\
    -Wl,--fatal-warnings \
    -fPIC \
    -Wl,-z,noexecstack \
    -Wl,-z,relro \
    -Wl,-z,now \
    -Wl,-z,defs \
    -Wl,--as-needed \
    -Wl,-O2 \
    -Wl,--gc-sections \
    -rdynamic \
    -L${SYSROOT_DIR}/usr/lib \
    -Wl,-rpath-link=${SYSROOT_DIR}/usr/lib \
    -pie \
    -Wl,-rpath-link=. \
    -Wl,--disable-new-dtags \
  ")  
  message(STATUS "linux compile...")
endif ()

SET(COMMON_LINK_OPTIONS "\
  -Wl,--export-dynamic \
") 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${RUNTIME_LIB_PATH}") 

# set abseil dir
set(abseil_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/abseil-cpp")

# set rtc_base dir
set(WEBRTC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(WEBRTC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(${WEBRTC_COMPILE_DEFINITIONS})

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WEBRTC_C_FLAGS} ${COMMON_CXX_FLAGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WEBRTC_CXX_FLAGS} ${COMMON_CXX_FLAGS}")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${WEBRTC_LINK_OPTIONS} ${COMMON_LINK_OPTIONS}")

# Process Abseil's CMake build system
add_subdirectory(${abseil_SOURCE_DIR})

add_subdirectory(system_wrappers)

add_subdirectory(rtc_base)


message (STATUS "#################libconfig info###############")  
message (STATUS "LIBCONFIG_INCLUDE_DIRS:${LIBCONFIG_INCLUDE_DIRS}")  
message (STATUS "LIBCONFIG_LIBRARIES:${LIBCONFIG_LIBRARIES}")  

message (STATUS "#################libopenssl info################")  
message (STATUS "OPENSSL_INCLUDE_DIRS:${OPENSSL_INCLUDE_DIRS}") 
message (STATUS "OPENSSL_LIBRARIES:${OPENSSL_LIBRARIES}")  

message (STATUS "#################libwebsockets info##############") 
message (STATUS "LIBWEBSOCKETS_INCLUDE_DIRS:${LIBWEBSOCKETS_INCLUDE_DIRS}")  
message (STATUS "LIBWEBSOCKETS_LIBRARIES:${LIBWEBSOCKETS_LIBRARIES}")  

message (STATUS "#################glib info#######################") 
message (STATUS "GLIB_INCLUDE_DIRS:${GLIB_INCLUDE_DIRS}") 
message (STATUS "GLIB_CONFIG_INCLUDE_DIRS:${GLIB_CONFIG_INCLUDE_DIRS}") 
message (STATUS "GLIB_LIBRARIES:${GLIB_LIBRARIES}")
message (STATUS "GIO_LIBRARIES:${GIO_LIBRARIES}")

message (STATUS "#################nice info########################")  
message (STATUS "NICE_LIBRARIES:${NICE_LIBRARIES}")  
message (STATUS "NICE_INCLUDE_DIRS:${NICE_INCLUDE_DIRS}") 
message (STATUS "##################################################")  

SET(CMAKE_REQUIRED_LIBRARIES "nice")
CHECK_FUNCTION_EXISTS(nice_address_equal_no_port HAVE_LIBNICE_TCP)
CHECK_FUNCTION_EXISTS(nice_agent_set_port_range HAVE_PORTRANGE)

message(STATUS "HAVE_LIBNICE_TCP:${HAVE_LIBNICE_TCP}")
message(STATUS "HAVE_PORTRANGE:${HAVE_PORTRANGE}")

if(${HAVE_LIBNICE_TCP})
	add_definitions(-DHAVE_LIBNICE_TCP=1)
endif()

if(${HAVE_PORTRANGE})
	add_definitions(-DHAVE_PORTRANGE=1)
endif()

#${OPENSSL_INCLUDE_DIRS}
include_directories(
	${WEBRTC_INCLUDE} 
  ${GLIB_INCLUDE_DIRS}
	${GLIB_CONFIG_INCLUDE_DIRS}
  ${NICE_INCLUDE_DIRS}
  ${LIBCONFIG_INCLUDE_DIRS}
  ${LIBWEBSOCKETS_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR})

set(GATEWAY_TRANSPORT 
  transports/transport.cc
  transports/websockets_transport.cc
  )

set(GATEWAY_BASE 
  async_queue.cc
  config.cc
  atomic.cc
  utils.cc
  error.cc
  helper.cc
)

add_executable(gateway
  ${GATEWAY_BASE}
  ${GATEWAY_TRANSPORT}  
  handle/handle.cc
	handle/nice_handle.cc 
	handle/p2p_handle.cc 
	plugins/p2p_call.cc 
	plugins/plugin.cc 
	jwt/base64.cc
	jwt/jwt.cc
	jwt/jwt-openssl.cc
	auth.cc
	cmdline.c
  gateway.cc
  main.cc
)

#dl
#rt
#z
#cap

target_link_libraries(  
  gateway
  rtc_base
	glib-2.0
	gio-2.0
	gobject-2.0
	nice
  config
	websockets
	#ssl
	#crypto 
	#${NICE_LIBRARIES}
  #${GLIB_LIBRARIES}
  #${LIBCONFIG_LIBRARIES}
  #${LIBWEBSOCKETS_LIBRARIES} 
  #${OPENSSL_LIBRARIES}
)

add_library(gateway_sdk STATIC
  ${GATEWAY_BASE}
  ${GATEWAY_TRANSPORT}
  sdk/transaction.cc
  sdk/gateway_client.cc
)

set_target_properties(gateway_sdk PROPERTIES OUTPUT_NAME "gateway_sdk")

target_link_libraries(
  gateway_sdk  
  rtc_base
  config
  websockets 
	ssl
	crypto
)

add_executable(
  gateway_client 
  client.cc
)  

target_link_libraries(
  gateway_client
  gateway_sdk
  rtc_base  
)

#for libwebsockets compile
#cmake -DCMAKE_INSTALL_PREFIX=/usr -DLWS_OPENSSL_INCLUDE_DIRS=/usr/include/ -DLWS_OPENSSL_LIBRARIES="/usr/lib/libssl.a;/usr/lib/libcrypto.a" -DLWS_WITH_BORINGSSL=ON -DLWS_WITHOUT_TEST_SERVER=OFF -DLWS_WITHOUT_TESTAPPS=OFF -DLWS_WITHOUT_TEST_PING=OFF -DLWS_WITHOUT_TEST_CLIENT=OFF -DCMAKE_SYSTEM_NAME="Linux" -DLWS_WITH_SHARED=OFF -DCMAKE_C_FLAGS="-fPIC -pthread" ..


