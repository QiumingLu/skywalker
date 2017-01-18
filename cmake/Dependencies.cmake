set(SkyWalker_LINKER_LIBS "")

find_package(LevelDB REQUIRED)
include_directories(SYSTEM ${LevelDB_INCLUDE_DIRS})
list(APPEND SkyWalker_LINKER_LIBS ${LevelDB_LIBRARIES})

find_package(Protobuf REQUIRED)
include_directories(SYSTEM ${PROTOBUF_INCLUDE_DIRS})
list(APPEND SkyWalker_LINKER_LIBS ${PROTOBUF_LIBRARIES})

find_package(Voyager REQUIRED)
include_directories(SYSTEM ${Voyager_INCLUDE_DIRS})
list(APPEND SkyWalker_LINKER_LIBS ${Voyager_LIBRARIES})
