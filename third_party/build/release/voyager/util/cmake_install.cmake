# Install script for directory: /Users/mirantslu/Development/skywalker/third_party/voyager/voyager/util

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/Users/mirantslu/Development/skywalker/third_party/build/release-install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/mirantslu/Development/skywalker/third_party/build/release/voyager/util/libvoyager_util.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libvoyager_util.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libvoyager_util.a")
    execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libvoyager_util.a")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/voyager/util" TYPE FILE FILES
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/util/logging.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/util/slice.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/util/status.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/util/stl_util.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/util/string_util.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/util/stringprintf.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/util/testharness.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/util/timeops.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/mirantslu/Development/skywalker/third_party/build/release/voyager/util/tests/cmake_install.cmake")

endif()

