set(Skywalker_LINKER_LIBS "")

include(cmake/Protobuf.cmake)

find_package(LevelDB REQUIRED)
include_directories(SYSTEM ${LEVELDB_INCLUDE_DIRS})
list(APPEND Skywalker_LINKER_LIBS ${LEVELDB_LIBRARIES})

find_package(Voyager REQUIRED)
include_directories(SYSTEM ${VOYAGER_INCLUDE_DIRS})
list(APPEND Skywalker_LINKER_LIBS ${VOYAGER_LIBRARIES})

find_package(Snappy)
if(SNAPPY_FOUND)
  include_directories(SYSTEM ${SNAPPY_INCLUDE_DIRS})
  list(APPEND Skywalker_LINKER_LIBS ${SNAPPY_LIBRARIES})
endif()

find_package(TCMalloc)
if(TCMALLOC_FOUND)
  include_directories(SYSTEM ${TCMALLOC_INCLUDE_DIRS})
  list(APPEND Skywalker_LINKER_LIBS ${TCMALLOC_LIBRARIES})
endif()

list(APPEND Skywalker_LINKER_LIBS pthread)
