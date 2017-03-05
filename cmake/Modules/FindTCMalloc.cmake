# - Find TCMalloc
#
# TCMALLOC_INCLUDE_DIRS  - List of TCMalloc includes.
# TCMALLOC_LIBRARIES     - List of libraries when using TCMalloc.
# TCMALLOC_FOUND         - True if TCMalloc found

# Look for the header of file.
find_path(TCMALLOC_INCLUDE NAMES google/tcmalloc.h
                           PATHS $ENV{TCMALLOC_ROOT}/include /opt/local/include /usr/local/include /usr/include
                           DOC "Path in which the file google/tcmalloc.h is located.")

# Look for the library.
find_library(TCMALLOC_LIBRARY NAMES tcmalloc
                              PATHS $ENV{TCMALLOC_ROOT}/lib /usr/local/lib /usr/lib
                              DOC "Path to tcmalloc library.")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TCMalloc DEFAULT_MSG TCMALLOC_INCLUDE TCMALLOC_LIBRARY)

if(TCMALLOC_FOUND)
  set(TCMALLOC_INCLUDE_DIRS ${TCMALLOC_INCLUDE})
  set(TCMALLOC_LIBRARIES ${TCMALLOC_LIBRARY})
  mark_as_advanced(TCMALLOC_INCLUDE TCMALLOC_LIBRARY)
  message(STATUS "Found TCMalloc (include: ${TCMALLOC_INCLUDE_DIRS}, library: ${TCMALLOC_LIBRARIES})")
endif()  
