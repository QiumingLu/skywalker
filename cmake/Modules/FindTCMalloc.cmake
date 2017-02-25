# - Find TCMalloc
#
# TCMalloc_INCLUDE_DIRS  - List of TCMalloc includes.
# TCMalloc_LIBRARIES     - List of libraries when using TCMalloc.
# TCMalloc_FOUND         - True if TCMalloc found

# Look for the header of file.
find_path(TCMalloc_INCLUDE NAMES google/tcmalloc.h
                           PATHS $ENV{TCMALLOC_ROOT}/include /opt/local/include /usr/local/include /usr/include
                           DOC "Path in which the file google/tcmalloc.h is located.")

# Look for the library.
find_library(TCMalloc_LIBRARY NAMES tcmalloc
                              PATHS $ENV{TCMALLOC_ROOT}/lib /usr/local/lib /usr/lib
                              DOC "Path to tcmalloc library.")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TCMalloc DEFAULT_MSG TCMalloc_INCLUDE TCMalloc_LIBRARY)

if(TCMalloc_FOUND)
  set(TCMalloc_INCLUDE_DIRS ${TCMalloc_INCLUDE})
  set(TCMalloc_LIBRARIES ${TCMalloc_LIBRARY})
  mark_as_advanced(TCMalloc_INCLUDE TCMalloc_LIBRARY)
  message(STATUS "Found TCMalloc (include: ${TCMalloc_INCLUDE_DIRS}, library: ${TCMalloc_LIBRARIES})")
endif()  
