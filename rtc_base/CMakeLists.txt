
cmake_minimum_required(VERSION 3.5)

if(NOT OPENSSL_INCLUDE_DIRS OR NOT OPENSSL_LIBRARIES)
  find_package(OpenSSL REQUIRED)
endif()

#############################################################################
# rtc_base_approved
if(WEBRTC_WIN)
  set(RTC_BASE_APPROVED_PLATFORM
  ${CMAKE_CURRENT_SOURCE_DIR}/win/windows_version.cc
  ) 
  set(RTC_BASE_APPROVED_LIB
    ${WEBRTC_WIN_RUNTIME_LIBS}
  )      
elseif(WEBRTC_ANDROID)
  set(RTC_BASE_APPROVED_LIB
    log
  )
endif()
  
set(RTC_BASE_APPROVED
  ${CMAKE_CURRENT_SOURCE_DIR}/bit_buffer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/buffer_queue.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/byte_buffer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/copy_on_write_buffer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/event_tracer.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/location.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/numerics/histogram_percentile_counter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/numerics/sample_counter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/race_checker.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/random.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/rate_statistics.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/rate_tracker.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/timestamp_aligner.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/zero_memory.cc
  ${RTC_BASE_APPROVED_PLATFORM}
  )  
#############################################################################  
# platform_thread_types
set(PLATFORM_THREAD_TYPES
  ${CMAKE_CURRENT_SOURCE_DIR}/platform_thread_types.cc
)     
#############################################################################  
# critical_section
set(CRITICAL_SECTION
  ${CMAKE_CURRENT_SOURCE_DIR}/critical_section.cc
) 
#############################################################################  
# platform_thread
set(PLATFORM_THREAD
  ${CMAKE_CURRENT_SOURCE_DIR}/platform_thread.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/thread_pool.cc
)      
#############################################################################  
# rtc_event
set(RTC_EVENT
  ${CMAKE_CURRENT_SOURCE_DIR}/event.cc
)

#############################################################################  
# logging  
set(RTC_LOGGING
 ${CMAKE_CURRENT_SOURCE_DIR}/logging.cc
)    
 
#############################################################################  
# checks
set(RTC_CHECKS
  ${CMAKE_CURRENT_SOURCE_DIR}/checks.cc
)
  
#############################################################################
# rate_limiter
set(RATE_LIMITER
  ${CMAKE_CURRENT_SOURCE_DIR}/rate_limiter.cc
)   
#############################################################################
#timeutils
set(TIME_UTILS
  ${CMAKE_CURRENT_SOURCE_DIR}/time_utils.cc
  )    
if(WEBRTC_WIN)
  set(TIME_UTILS_LIB
    winmm.lib
  )      
endif()
#############################################################################
#stringutils
set(STRING_UTILS
  ${CMAKE_CURRENT_SOURCE_DIR}/string_encode.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/string_to_number.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/string_utils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/strings/string_builder.cc
) 
#############################################################################
  
if (WEBRTC_POSIX OR WEBRTC_LINUX) 
  set(RTC_ENABLE_LIBEVENT 1)
endif()

# rtc task queue
if (RTC_ENABLE_LIBEVENT)
  set(RTC_BASE_TASK_QUEUE_LIBEVENT
    ${CMAKE_CURRENT_SOURCE_DIR}/task_queue_libevent.cc
    ${WEBRTC_DIR}/api/task_queue/default_task_queue_factory_libevent.cc
  )
  if (NOT LIBEVENT_INCLUDE_DIRS OR NOT LIBEVENT_LIBRARIES)
    find_package(Libevent)
  endif()
  set(EVENT_LIBS ${LIBEVENT_LIBRARIES})
else()
  if (WEBRTC_IOS OR WEBRTC_MAC)
    set(RTC_BASE_TASK_QUEUE_GCD
      ${CMAKE_CURRENT_SOURCE_DIR}/task_queue_gcd.cc
      ${WEBRTC_DIR}/api/task_queue/default_task_queue_factory_gcd.cc
    )  
  elseif(WEBRTC_WIN)
    set(RTC_BASE_TASK_QUEUE_WIN
      ${CMAKE_CURRENT_SOURCE_DIR}/task_queue_win.cc
      ${WEBRTC_DIR}/api/task_queue/default_task_queue_factory_win.cc
    )  
  else()
    set(RTC_BASE_TASK_QUEUE_STDLIB
      ${CMAKE_CURRENT_SOURCE_DIR}/task_queue_stdlib.cc
      ${WEBRTC_DIR}/api/task_queue/default_task_queue_factory_stdlib.cc
    )    
  endif()
endif()

message(STATUS "EVENT_LIBS:${EVENT_LIBS}")

set(RTC_BASE_TASK_QUEUE
  ${CMAKE_CURRENT_SOURCE_DIR}/task_queue.cc 
  ${WEBRTC_DIR}/api/task_queue/task_queue_base.cc
  ${RTC_BASE_TASK_QUEUE_LIBEVENT}
  ${RTC_BASE_TASK_QUEUE_GCD}
  ${RTC_BASE_TASK_QUEUE_WIN}
  ${RTC_BASE_TASK_QUEUE_STDLIB}
)

#############################################################################
# weak ptr
set(WEAK_PTR
  ${CMAKE_CURRENT_SOURCE_DIR}/weak_ptr.cc
) 

#############################################################################
# rtc_numerics
set(RTC_NUMERICS
  ${CMAKE_CURRENT_SOURCE_DIR}/numerics/exp_filter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/numerics/moving_average.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/numerics/samples_stats_counter.cc
) 
#############################################################################
# rtc_json
if(NOT JSONCPP_INCLUDE_DIRS OR NOT JSONCPP_LIBRARIES)
find_package (Jsoncpp REQUIRED)
endif()

set(RTC_JSON
  ${CMAKE_CURRENT_SOURCE_DIR}/strings/json.cc
) 
#add_definitions(-DWEBRTC_EXTERNAL_JSON)   
#############################################################################

if(WEBRTC_WIN)
  set(RTC_BASE_PLATFORM
    ${CMAKE_CURRENT_SOURCE_DIR}/win32_socket_server.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/win32.cc,
    ${CMAKE_CURRENT_SOURCE_DIR}/win32_window.cc
  )   
  set(RTC_BASE_LIB
    crypt32.lib
    iphlpapi.lib
    secur32.lib
  )  

elseif(WEBRTC_ANDROID)  
  set(RTC_BASE_PLATFORM
    ${CMAKE_CURRENT_SOURCE_DIR}/ifaddrs_android.cc
  )   
  set(RTC_BASE_LIB
    log
    GLESv2
  ) 
elseif (WEBRTC_IOS OR WEBRTC_MAC)
  set(RTC_BASE_PLATFORM
    ${CMAKE_CURRENT_SOURCE_DIR}/mac_ifaddrs_converter.cc
  )  
  if (WEBRTC_IOS) 
    set(RTC_BASE_LIB
      CFNetwork.framework
      Foundation.framework
      Security.framework
      SystemConfiguration.framework
      UIKit.framework
    ) 
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)   
  endif()
elseif (WEBRTC_LINUX)  
  set(RTC_BASE_LIB
    dl
    rt
  ) 
endif()   

if (WEBRTC_POSIX OR WEBRTC_FUCHSIA) 
  set(RTC_BASE_PLATFORM
    ${CMAKE_CURRENT_SOURCE_DIR}/ifaddrs_converter.cc
  )  
endif()

if (WEBRTC_NACL) 
  set(RTC_BASE_PLATFORM
    ${CMAKE_CURRENT_SOURCE_DIR}/ifaddrs_converter.cc
  )  
  add_definitions(-Dtimezone=_timezone) 
endif()

# rtc_base
set(RTC_BASE
  ${CMAKE_CURRENT_SOURCE_DIR}/async_invoker.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/async_packet_socket.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/async_resolver_interface.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/async_socket.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/async_tcp_socket.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/async_udp_socket.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/crc32.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/crypt_string.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/data_rate_limiter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/file_rotating_stream.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/helpers.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/http_common.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ip_address.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/message_digest.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/message_handler.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/message_queue.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/net_helper.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/net_helpers.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/network.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/network_monitor.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/null_socket_server.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/openssl_adapter.cc  
  ${CMAKE_CURRENT_SOURCE_DIR}/openssl_certificate.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/openssl_digest.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/openssl_identity.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/openssl_session_cache.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/openssl_stream_adapter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/openssl_utility.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/physical_socket_server.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/proxy_info.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/rtc_certificate.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/rtc_certificate_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/signal_thread.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/socket.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/socket_adapters.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/socket_address.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/socket_address_pair.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ssl_adapter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ssl_certificate.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ssl_fingerprint.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ssl_identity.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ssl_stream_adapter.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/stream.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/thread.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/unique_id_generator.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/log_sinks.cc
  ${RTC_BASE_PLATFORM}
)  

if(WEBRTC_WIN)
  set(TIME_UTILS_LIB
    winmm.lib
  ) 
endif()   
#############################################################################
#system
set(SYSTEM 
  ${CMAKE_CURRENT_SOURCE_DIR}/system/thread_registry.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/system/file_wrapper.cc
) 
#############################################################################  
# synchronization 
if (WEBRTC_WIN)    
  set(SYNCHRONIZATION_PLATFORM 
    ${CMAKE_CURRENT_SOURCE_DIR}/synchronization/rw_lock_win.cc
  ) 
else()
  set(SYNCHRONIZATION_PLATFORM 
    ${CMAKE_CURRENT_SOURCE_DIR}/synchronization/rw_lock_posix.cc
  ) 
endif()

set(SYNCHRONIZATION 
  ${CMAKE_CURRENT_SOURCE_DIR}/synchronization/rw_lock_wrapper.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/synchronization/rw_lock_posix.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/synchronization/sequence_checker.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/synchronization/yield_policy.cc
  ${SYNCHRONIZATION_PLATFORM}
)   
#############################################################################   
#network
set(NET_WORK 
  ${CMAKE_CURRENT_SOURCE_DIR}/network/sent_packet.cc
) 
#############################################################################     
#memory
set(MEMORY 
  ${CMAKE_CURRENT_SOURCE_DIR}/memory/aligned_malloc.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/memory/fifo_buffer.cc
) 
############################################################################# 
# task utils
set(TASK_UTILS 
  ${CMAKE_CURRENT_SOURCE_DIR}/task_utils/repeating_task.cc
) 
############################################################################# 
# third_party
set(THIRD_PARTY 
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/base64/base64.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sigslot/sigslot.cc
) 
#############################################################################
# units
set (UNITE
 ${WEBRTC_DIR}/api/units/data_rate.cc
 ${WEBRTC_DIR}/api/units/data_size.cc
 ${WEBRTC_DIR}/api/units/time_delta.cc
 ${WEBRTC_DIR}/api/units/frequency.cc
 ${WEBRTC_DIR}/api/units/timestamp.cc
)

#############################################################################
set (WEBRTC_INCLUDE 
	${WEBRTC_INCLUDE}
	${OPENSSL_INCLUDE_DIRS}
  ${LIBEVENT_INCLUDE_DIRS}
  ${JSONCPP_INCLUDE_DIRS}
)

#include_directories(
#  ${WEBRTC_INCLUDE}
#)

if(NOT OPENSSL_INCLUDE_DIRS OR NOT OPENSSL_LIBRARIES)
  find_package(openssl REQUIRED)
endif()

#添加静态库  
add_library(rtc_base STATIC 
  ${RTC_BASE_APPROVED}
  ${PLATFORM_THREAD_TYPES} 
  ${CRITICAL_SECTION} 
  ${PLATFORM_THREAD}
  ${RTC_EVENT}
  ${RTC_LOGGING}  
  ${RTC_CHECKS} 
  ${RATE_LIMITER} 
  ${TIME_UTILS} 
  ${STRING_UTILS} 
  ${RTC_BASE_TASK_QUEUE}
  ${WEAK_PTR}
  ${RTC_NUMERICS}
  ${RTC_JSON}
  ${RTC_BASE}
  ${SYNCHRONIZATION}  
  ${NET_WORK}
  ${SYSTEM}
  ${MEMORY}
  ${TASK_UTILS}
  ${THIRD_PARTY}
  ${UNITE}
)

#将静态库重新命名为librtc_base.a
set_target_properties(rtc_base PROPERTIES OUTPUT_NAME "rtc_base")
#设置头文件路径  
target_include_directories(
  rtc_base 
  PUBLIC 
  ${WEBRTC_INCLUDE}
)

# Declare dependency on the absl::strings library
target_link_libraries(rtc_base
  absl::base
  absl::algorithm
  absl::flat_hash_map
  absl::memory
  #absl::meta
  absl::numeric
  absl::strings
  absl::synchronization
  #absl::time
  #absl::utility
  ${OPENSSL_LIBRARIES}
  #${LIBCONFIG_LIBRARIES}
  ${RTC_BASE_APPROVED_LIB}
  ${TIME_UTILS_LIB}
  ${RTC_BASE_LIB}  
  ${EVENT_LIBS}
  ${JSONCPP_LIBRARIES}
  system_wrappers
	#ssl
	#crypto
  atomic 
  pthread 
)
