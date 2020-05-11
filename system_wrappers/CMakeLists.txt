cmake_minimum_required(VERSION 3.5)

include_directories(${WEBRTC_INCLUDE})

if (WEBRTC_LINUX)
  set(CPU_FEATURES_LINUX
    ${CMAKE_CURRENT_SOURCE_DIR}/source/cpu_features_linux.c
  )
  
  add_library(
    cpu_features_linux 
    STATIC 
    ${CPU_FEATURES_LINUX}
  )
  
  set_target_properties(cpu_features_linux PROPERTIES OUTPUT_NAME "cpu_features_linux")
  
  target_link_libraries(
    cpu_features_linux
    rtc_base
  )  
  
endif()

if (WEBRTC_ANDROID)
  set(CPU_FEATURES_ANDROID
    ${CMAKE_CURRENT_SOURCE_DIR}/source/cpu_features_android.c
  )
  
  add_library(
    cpu_features_android 
    STATIC 
    ${CPU_FEATURES_ANDROID}
  )
  
  set_target_properties(cpu_features_android PROPERTIES OUTPUT_NAME "cpu_features_android")
  
  target_link_libraries(
    cpu_features_linux
    ${ANDROID_CPU_FEATURES}
  )
endif()

set(SYSTEM_WRAPPERS
  ${CMAKE_CURRENT_SOURCE_DIR}/source/clock.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/cpu_features.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/cpu_info.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/cpu_features.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/source/sleep.cc
)

#添加静态库  
add_library(system_wrappers STATIC 
  ${SYSTEM_WRAPPERS}
)

if (WEBRTC_LINUX)
  set(SYSTEM_WRAPPERS_LIB
    cpu_features_linux
    rt
  )
elseif(WEBRTC_WIN)
  set(SYSTEM_WRAPPERS_LIB
    winmm.lib
  )
elseif(WEBRTC_ANDROID)
  set(SYSTEM_WRAPPERS_LIB
    log
    cpu_features_android
  )
endif()

set_target_properties(system_wrappers PROPERTIES OUTPUT_NAME "system_wrappers")

target_link_libraries(system_wrappers
  rtc_base
  ${SYSTEM_WRAPPERS_LIB}
)


