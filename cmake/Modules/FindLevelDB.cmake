# - Find LevelDB
#
# LEVELDB_INCLUDE_DIRS  - List of LevelDB includes.
# LEVELDB_LIBRARIES     - List of libraries when using LevelDB.
# LEVELDB_FOUND         - True if LevelDB found

# Look for the header of file.
find_path(LEVELDB_INCLUDE NAMES leveldb/db.h
                          PATHS $ENV{LEVELDB_ROOT}/include /opt/local/include /usr/local/include /usr/include
                          DOC "Path in which the file leveldb/db.h is located.")

# Look for the library.
find_library(LEVELDB_LIBRARY NAMES leveldb
                             PATHS $ENV{LEVELDB_ROOT}/lib /usr/local/lib /usr/lib
                             DOC "Path to leveldb library.")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LevelDB DEFAULT_MSG LEVELDB_INCLUDE LEVELDB_LIBRARY)

if(LEVELDB_FOUND)
  set(LEVELDB_INCLUDE_DIRS ${LEVELDB_INCLUDE})
  set(LEVELDB_LIBRARIES ${LEVELDB_LIBRARY})
  mark_as_advanced(LEVELDB_INCLUDE LEVELDB_LIBRARY)
  message(STATUS "Found LevelDB (include: ${LEVELDB_INCLUDE_DIRS}, library: ${LEVELDB_LIBRARIES})")
endif()  
