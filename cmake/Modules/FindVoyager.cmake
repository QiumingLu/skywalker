# - Find Voyager
#
# Voyager_INCLUDE_DIRS  - List of Voyager includes.
# Voyager_LIBRARIES     - List of libraries when using Voyager.
# Voyager_FOUND         - True if Voyager found

# Look for the header of file.
find_path(Voyager_INCLUDE NAMES voyager/core/tcp_server.h
                          PATHS $ENV{VOYAGER_ROOT}/include /opt/local/include /usr/local/include /usr/include
                                ${PROJECT_SOURCE_DIR}/third_party/voyager/build/release-install/include
                                ${PROJECT_SOURCE_DIR}/third_party/voyager/build/debug-install/include
                          DOC "Path in which the file voyager/core/tcp_server.h is located.")
                                
# Look for the library.
find_library(Voyager_UTIL_LIBRARY NAMES voyager_util
                             PATHS $ENV{VOYAGER_ROOT}/lib /usr/local/lib /usr/lib
                                   ${PROJECT_SOURCE_DIR}/third_party/voyager/build/release-install/lib
                                   ${PROJECT_SOURCE_DIR}/third_party/voyager/build/debug-install/lib
                             DOC "Path to voyager_util library.")

find_library(Voyager_PORT_LIBRARY NAMES voyager_port
                             PATHS $ENV{VOYAGER_ROOT}/lib /usr/local/lib /usr/lib
                                   ${PROJECT_SOURCE_DIR}/third_party/voyager/build/release-install/lib
                                   ${PROJECT_SOURCE_DIR}/third_party/voyager/build/debug-install/lib
                             DOC "Path to voyager_port library.")
                           
find_library(Voyager_CORE_LIBRARY NAMES voyager_core
                             PATHS $ENV{VOYAGER_ROOT}/lib /usr/local/lib /usr/lib
                                   ${PROJECT_SOURCE_DIR}/third_party/voyager/build/release-install/lib
                                   ${PROJECT_SOURCE_DIR}/third_party/voyager/build/debug-install/lib
                             DOC "Path to voyager_core library.")
              
find_library(Voyager_RPC_LIBRARY NAMES voyager_rpc
                             PATHS $ENV{VOYAGER_ROOT}/lib /usr/local/lib /usr/lib
                                   ${PROJECT_SOURCE_DIR}/third_party/voyager/build/release-install/lib
                                   ${PROJECT_SOURCE_DIR}/third_party/voyager/build/debug-install/lib
                             DOC "Path to voyager_rpc library.")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Voyager DEFAULT_MSG Voyager_INCLUDE Voyager_UTIL_LIBRARY Voyager_PORT_LIBRARY Voyager_CORE_LIBRARY Voyager_RPC_LIBRARY)

if(LEVELDB_FOUND)
  set(Voyager_INCLUDE_DIRS ${Voyager_INCLUDE})
  set(Voyager_LIBRARIES ${Voyager_UTIL_LIBRARY} ${Voyager_PORT_LIBRARY} ${Voyager_CORE_LIBRARY} ${Voyager_RPC_LIBRARY})
  mark_as_advanced(Voyager_INCLUDE Voyager_LIBRARY)
  message(STATUS "Found Voyager (include: ${Voyager_INCLUDE}, library: ${Voyager_LIBRARIES})")
endif()  
