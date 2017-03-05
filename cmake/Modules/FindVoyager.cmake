# - Find Voyager
#
# VOYAGER_INCLUDE_DIRS  - List of Voyager includes.
# VOYAGER_LIBRARIES     - List of libraries when using Voyager.
# VOYAGER_FOUND         - True if Voyager found

# Look for the header of file.
find_path(VOYAGER_INCLUDE NAMES voyager/core/tcp_server.h
                          PATHS $ENV{VOYAGER_ROOT}/include /opt/local/include /usr/local/include /usr/include
                          DOC "Path in which the file voyager/core/tcp_server.h is located.")
                                
# Look for the library.
find_library(VOYAGER_UTIL_LIBRARY NAMES voyager_util
                                  PATHS $ENV{VOYAGER_ROOT}/lib /usr/local/lib /usr/lib
                                  DOC "Path to voyager_util library.")

find_library(VOYAGER_PORT_LIBRARY NAMES voyager_port
                                  PATHS $ENV{VOYAGER_ROOT}/lib /usr/local/lib /usr/lib
                                  DOC "Path to voyager_port library.")
                           
find_library(VOYAGER_CORE_LIBRARY NAMES voyager_core
                                  PATHS $ENV{VOYAGER_ROOT}/lib /usr/local/lib /usr/lib
                                  DOC "Path to voyager_core library.")
              
find_library(VOYAGER_RPC_LIBRARY NAMES voyager_rpc
                                 PATHS $ENV{VOYAGER_ROOT}/lib /usr/local/lib /usr/lib
                                 DOC "Path to voyager_rpc library.")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Voyager DEFAULT_MSG VOYAGER_INCLUDE VOYAGER_UTIL_LIBRARY VOYAGER_PORT_LIBRARY VOYAGER_CORE_LIBRARY VOYAGER_RPC_LIBRARY)

if(VOYAGER_FOUND)
  set(VOYAGER_INCLUDE_DIRS ${VOYAGER_INCLUDE})
  set(VOYAGER_LIBRARIES 
      ${VOYAGER_UTIL_LIBRARY}
      ${VOYAGER_PORT_LIBRARY}
      ${VOYAGER_CORE_LIBRARY} 
      ${VOYAGER_RPC_LIBRARY}
     )
  mark_as_advanced(VOYAGER_INCLUDE VOYAGER_LIBRARY)
  message(STATUS "Found Voyager (include: ${VOYAGER_INCLUDE_DIRS}, library: ${VOYAGER_LIBRARIES})")
endif()  
