cmake_minimum_required(VERSION 2.8.7)

if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()

project (Skywalker C CXX)

set(SKYWALKER_VERSION_MAJOR 1)
set(SKYWALKER_VERSION_MINOR 2)
set(SKYWALKER_VERSION_STRING ${SKYWALKER_VERSION_MAJOR}.${SKYWALKER_VERSION_MINOR})

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "release")
endif()

if(NOT CMAKE_INSTALL_DIR)
  set(CMAKE_INSTALL_DIR "/usr/local")
endif()

option(BUILD_EXAMPLES "Build skywalker examples" off)
option(BUILD_SHARED_LIBS "Build skywalker shared libraries" on)

set(CXX_FLAGS
  -g
  -DCHECK_PTHREAD_RETURN_VALUE
  -D_FILE_OFFSET_BITS=64
  -Wall
  -Wextra
  #  -Werror
  -Wconversion
  -Wno-unused-parameter
  -Woverloaded-virtual
  -Wpointer-arith
  -Wwrite-strings
  -Wshadow
  -march=native
  -std=c++11
  -Wno-sign-conversion
  )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  list(APPEND CXX_FLAGS "-rdynamic")
endif()

if(CMAKE_BUILD_BITS EQUAL 32)
  list(APPEND CXX_FLAGS "-m32")
endif()

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_COMPILER "g++")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_COMPILER "clang++")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBARARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

set(Skywalker_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories(${Skywalker_INCLUDE_DIR} ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

include(cmake/Dependencies.cmake)
include(cmake/Targets.cmake)

message(STATUS ${Skywalker_LINKER_LIBS})

skywalker_set_skywalker_link()
if(BUILD_SHARED_LIBS)
  add_library(skywalker SHARED ${skywalker_srcs})
  add_library(skywalker_static STATIC ${skywalker_srcs})
  set_target_properties(skywalker_static PROPERTIES OUTPUT_NAME "skywalker")
  set_target_properties(skywalker PROPERTIES CLEAN_DIRECT_OUTPUT ${SKYWALKER_VERSION_MAJOR})
  set_target_properties(skywalker_static PROPERTIES CLEAN_DIRECT_OUTPUT ${SKYWALKER_VERSION_MAJOR})
  set_target_properties(skywalker PROPERTIES VERSION ${SKYWALKER_VERSION_STRING} SOVERSION ${SKYWALKER_VERSION_MAJOR})
  target_link_libraries(skywalker ${Skywalker_LINKER_LIBS})
  install(TARGETS skywalker DESTINATION lib)
  install(TARGETS skywalker_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
else()
  add_library(skywalker ${skywalker_srcs})
  target_link_libraries(skywalker ${Skywalker_LINKER_LIBS})
  install(TARGETS skywalker DESTINATION lib)
endif()

install(DIRECTORY ${Skywalker_INCLUDE_DIR}/skywalker DESTINATION include)

if (BUILD_EXAMPLES)
  add_subdirectory(examples/journey)
  #  add_subdirectory(paxos/test)
endif()
