#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "double-conversion::double-conversion" for configuration ""
set_property(TARGET double-conversion::double-conversion APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(double-conversion::double-conversion PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libdouble-conversion.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS double-conversion::double-conversion )
list(APPEND _IMPORT_CHECK_FILES_FOR_double-conversion::double-conversion "${_IMPORT_PREFIX}/lib/libdouble-conversion.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
