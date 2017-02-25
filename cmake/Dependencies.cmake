set(SkyWalker_LINKER_LIBS "")

include(cmake/Protobuf.cmake)

find_package(LevelDB REQUIRED)
include_directories(SYSTEM ${LevelDB_INCLUDE_DIRS})
list(APPEND SkyWalker_LINKER_LIBS ${LevelDB_LIBRARIES})

find_package(Voyager REQUIRED)
include_directories(SYSTEM ${Voyager_INCLUDE_DIRS})
list(APPEND SkyWalker_LINKER_LIBS ${Voyager_LIBRARIES})

find_package(TCMalloc)
if(TCMALLOC_FOUND)
  include_directories(SYSTEM ${TCMalloc_INCLUDE_DIRS})
  list(APPEND SkyWalker_LINKER_LIBS ${TCMalloc_LIBRARIES})
endif()

list(APPEND SkyWalker_LINKER_LIBS pthread)
