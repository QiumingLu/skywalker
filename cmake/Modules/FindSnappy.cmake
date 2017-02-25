# - Find Snappy
#
# Snappy_INCLUDE_DIRS  - List of Snappy includes.
# Snappy_LIBRARIES     - List of libraries when using Snappy.
# Snappy_FOUND         - True if Snappy found

# Look for the header of file.
find_path(Snappy_INCLUDE NAMES snappy.h
                          PATHS $ENV{SNAPPY_ROOT}/include /opt/local/include /usr/local/include /usr/include
                          DOC "Path in which the file snappy.h is located.")

# Look for the library.
find_library(Snappy_LIBRARY NAMES snappy
                            PATHS $ENV{SNAPPY_ROOT}/lib /usr/local/lib /usr/lib
                            DOC "Path to snappy library.")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Snappy DEFAULT_MSG Snappy_INCLUDE Snappy_LIBRARY)

if(Snappy_FOUND)
  set(Snappy_INCLUDE_DIRS ${Snappy_INCLUDE})
  set(Snappy_LIBRARIES ${Snappy_LIBRARY})
  mark_as_advanced(Snappy_INCLUDE Snappy_LIBRARY)
  message(STATUS "Found Snappy (include: ${Snappy_INCLUDE_DIRS}, library: ${Snappy_LIBRARIES})")
endif()  
