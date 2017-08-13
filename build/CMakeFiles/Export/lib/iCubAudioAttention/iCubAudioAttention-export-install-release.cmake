#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "audioLib" for configuration "Release"
set_property(TARGET audioLib APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(audioLib PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "YARP::YARP_OS;YARP::YARP_sig;YARP::YARP_math;YARP::YARP_dev;YARP::YARP_name;YARP::YARP_init"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libaudioLib.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS audioLib )
list(APPEND _IMPORT_CHECK_FILES_FOR_audioLib "${_IMPORT_PREFIX}/lib/libaudioLib.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
