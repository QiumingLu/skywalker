cmake_minimum_required(VERSION 3.0)

project (SkyWalker C CXX)

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
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
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

set(SkyWalker_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories(${SkyWalker_INCLUDE_DIR} ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

include(cmake/Dependencies.cmake)
include(cmake/Targets.cmake)

message(STATUS ${SkyWalker_LINKER_LIBS})

skywalker_set_skywalker_link()
#add_library(skywalker SHARED ${skywalker_SRCS})
#add_library(skywalker_static STATIC ${skywalker_SRCS})
#set_target_properties(skywalker_static PROPERTIES OUTPUT_NAME "skywalker")
#set_target_properties(skywalker PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(skywalker_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties(skywalker PROPERTIES VERSION 1.0 SOVERSION 1)
add_library(skywalker ${skywalker_SRCS})
target_link_libraries(skywalker ${SkyWalker_LINKER_LIBS})
install(DIRECTORY ${SkyWalker_INCLUDE_DIR}/skywalker DESTINATION include)
install(TARGETS skywalker DESTINATION lib)
#install(TARGETS skywalker_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
  add_subdirectory(examples/journey)
  add_subdirectory(paxos/test)
endif()
