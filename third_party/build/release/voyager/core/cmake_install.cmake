# Install script for directory: /Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/mirantslu/Development/skywalker/third_party/build/release/voyager/core/libvoyager_core.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libvoyager_core.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libvoyager_core.a")
    execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libvoyager_core.a")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/voyager/core" TYPE FILE FILES
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/base_socket.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/bg_eventloop.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/buffer.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/callback.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/client_socket.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/dispatch.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/eventloop.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/event_poll.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/event_select.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/schedule.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/server_socket.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/sockaddr.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/tcp_client.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/tcp_connection.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/tcp_server.h"
    "/Users/mirantslu/Development/skywalker/third_party/voyager/voyager/core/timerlist.h"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/mirantslu/Development/skywalker/third_party/build/release/voyager/core/tests/cmake_install.cmake")

endif()

