set(SkyWalker_LINKER_LIBS "")

include(cmake/Protobuf.cmake)

find_package(LevelDB REQUIRED)
include_directories(SYSTEM ${LevelDB_INCLUDE_DIRS})
list(APPEND SkyWalker_LINKER_LIBS ${LevelDB_LIBRARIES})

find_package(Voyager REQUIRED)
include_directories(SYSTEM ${Voyager_INCLUDE_DIRS})
list(APPEND SkyWalker_LINKER_LIBS ${Voyager_LIBRARIES})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  list(APPEND SkyWalker_LINKER_LIBS pthread rt)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  list(APPEND SkyWalker_LINKER_LIBS pthread)
endif()
