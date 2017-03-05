# - Find Snappy
#
# SNAPPY_INCLUDE_DIRS  - List of Snappy includes.
# SNAPPY_LIBRARIES     - List of libraries when using Snappy.
# SNAPPY_FOUND         - True if Snappy found

# Look for the header of file.
find_path(SNAPPY_INCLUDE NAMES snappy.h
                          PATHS $ENV{SNAPPY_ROOT}/include /opt/local/include /usr/local/include /usr/include
                          DOC "Path in which the file snappy.h is located.")

# Look for the library.
find_library(SNAPPY_LIBRARY NAMES snappy
                            PATHS $ENV{SNAPPY_ROOT}/lib /usr/local/lib /usr/lib
                            DOC "Path to snappy library.")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Snappy DEFAULT_MSG SNAPPY_INCLUDE SNAPPY_LIBRARY)

if(SNAPPY_FOUND)
  set(SNAPPY_INCLUDE_DIRS ${SNAPPY_INCLUDE})
  set(SNAPPY_LIBRARIES ${SNAPPY_LIBRARY})
  mark_as_advanced(SNAPPY_INCLUDE SNAPPY_LIBRARY)
  message(STATUS "Found Snappy (include: ${SNAPPY_INCLUDE_DIRS}, library: ${SNAPPY_LIBRARIES})")
endif()  
