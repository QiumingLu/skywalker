cmake_minimum_required(VERSION 2.8.12)

if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()

project (Skywalker C CXX)

set(SKYWALKER_VERSION_MAJOR 1)
set(SKYWALKER_VERSION_MINOR 0)
set(SKYWALKER_VERSION_PATCH 3)
set(SKYWALKER_VERSION_STRING ${SKYWALKER_VERSION_MAJOR}.${SKYWALKER_VERSION_MINOR}.${SKYWALKER_VERSION_PATCH})

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build : None Debug Release RelWithDebInfo MinSizeRel Coverage." FORCE)
endif()

if(NOT CMAKE_INSTALL_DIR)
  set(CMAKE_INSTALL_DIR "/usr/local")
endif()

option(BUILD_EXAMPLES "Build skywalker examples" ON)
option(BUILD_SHARED_LIBS "Build skywalker shared libraries" ON)

set(CXX_FLAGS
  -g
  -Wall
  -Wextra
  -Wconversion
  -Wno-unused-parameter
  -Woverloaded-virtual
  -Wpointer-arith
  -Wwrite-strings
  -Wshadow
  -Wno-sign-conversion
  -std=c++14
)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

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
  add_subdirectory(examples/echo)
  add_subdirectory(examples/journey)
  add_subdirectory(paxos/tests)
endif()
