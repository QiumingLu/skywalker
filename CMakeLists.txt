cmake_minimum_required(VERSION 3.0)

project (skywalker C CXX)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "release")
endif()

set(CXX_FLAGS
  -g
  -DCHECK_PTHREAD_RETURN_VALUE
  -D_FILE_OFFSET_BITS=64
  -Wall
  -Wextra
  -Werror
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
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_COMPILER "clang++")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBARARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

set(VOYAGER_PATH ${PROJECT_SOURCE_DIR}/third_party/voyager)
set(VOYAGER_INCLUDE_DIR ${PROTOBUF_PATH}/include)
set(VOYAGER_LIBRARY_DIR ${PROTOBUF_PATH}/lib)
include_directories(${VOYAGER_INCLUDE_DIR})
link_directories(${VOYAGER_LIBRARY_DIR})

message(STATUS "VOYAGER_PATH = " ${VOYAGER_PATH})
message(STATUS "VOYAGER_INCLUDE_DIR = " ${VOYAGER_INCLUDE_DIR})
message(STATUS "VOYAGER_LIBRARY_DIR = " ${VOYAGER_LIBRARY_DIR})


set(LEVELDB_PATH ${PROJECT_SOURCE_DIR}/third_party/leveldb)
set(LEVELDB_INCLUDE_DIR ${LEVELDB_PATH}/include)
set(LEVELDB_LIBRARY_DIR ${LEVELDB_PATH}/lib)
include_directories(${LEVELDB_INCLUDE_DIR})
link_directories(${LEVELDB_LIBRARY_DIR})

message(STATUS "LEVELDB_PATH = " ${LEVELDB_PATH})
message(STATUS "LEVELDB_INCLUDE_DIR = " ${LEVELDB_INCLUDE_DIR})
message(STATUS "LEVELDB_LIBRARY_DIR = " ${LEVELDB_LIBRARY_DIR})

set(PROTOBUF_PATH ${PROJECT_SOURCE_DIR}/third_party/protobuf)
set(PROTOBUF_INCLUDE_DIR ${PROTOBUF_PATH}/include)
set(PROTOBUF_LIBRARY_DIR ${PROTOBUF_PATH}/lib)
include_directories(${PROTOBUF_INCLUDE_DIR})
link_directories(${PROTOBUF_LIBRARY_DIR})

message(STATUS "PROTOBUF_PATH = " ${PROTOBUF_PATH})
message(STATUS "PROTOBUF_INCLUDE_DIR = " ${PROTOBUF_INCLUDE_DIR})
message(STATUS "PROTOBUF_LIBRARY_DIR = " ${PROTOBUF_LIBRARY_DIR})

add_subdirectory(skywalker/util)
add_subdirectory(skywalker/network)
add_subdirectory(skywalker/storage)
add_subdirectory(skywalker/paxos)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
  add_subdirectory(examples)
  add_subdirectory(examples)
  add_subdirectory(benchmarks)
endif()
