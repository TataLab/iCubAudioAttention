# Copyright: (C) 2019 Department of Neuroscience - University of Lethbridge
# Authors: Austin Kothig <kothiga@uleth.ca>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 3.5)

project(AudioAttention LANGUAGES CXX C VERSION 1.0.0)
message("PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

############################################################

include(GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_EXTENSIONS        OFF)
set(CMAKE_CXX_STANDARD          11)
set(CMAKE_CXX_STANDARD_REQUIRED 11)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

############################################################

find_package(ICUBcontrib REQUIRED)
find_package(YCM         REQUIRED)
find_package(YARP        REQUIRED)
find_package(ICUB        REQUIRED) 

list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${YCM_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUB_MODULE_PATH})

include(ICUBcontribOptions)
include(ICUBcontribHelpers)

############################################################

option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" OFF)

include(AddInstallRPATHSupport)
add_install_rpath_support(
    BIN_DIRS          "${CMAKE_INSTALL_FULL_LIBDIR}"
    LIB_DIRS          "${CMAKE_INSTALL_FULL_BINDIR}"
    INSTALL_NAME_DIR  "${CMAKE_INSTALL_FULL_LIBDIR}"
    USE_LINK_PATH
)

if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif()
endif()

############################################################

icubcontrib_set_default_prefix()

add_subdirectory(audioCubLib)

include(AddUninstallTarget)

############################################################

option(ENABLE_COMPILE_ON_PC104  "Are you compiling on PC104?" OFF)

add_subdirectory( dependencies )
add_subdirectory( modules      )
add_subdirectory( app          )

############################################################







#  if(COMMAND cmake_policy)
#      cmake_policy(SET CMP0003 NEW)
#  endif(COMMAND cmake_policy)
#  
#  project (audioAttention)
#  
#  # Check for the minimal set of packages required for the global project.
#  # If we include them here, sub-projects can be configured a bit faster.
#  # Only a few demo projects are included so far.
#  
#  find_package(YARP REQUIRED)
#  find_package(ICUB REQUIRED)
#  list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
#  list(APPEND CMAKE_MODULE_PATH ${ICUB_MODULE_PATH})
#  include(YarpInstallationHelpers)
#  
#  
#  find_package(ICUBcontrib REQUIRED)
#  list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
#  
#  set(CMAKE_MODULE_PATH 
#    ${CMAKE_MODULE_PATH} 
#    ${YARP_MODULE_PATH} 
#    ${ICUB_MODULE_PATH} 
#    ${ICUBCONTRIB_MODULE_PATH} # note 10/10/13 aligned to the standard Rea
#  ) 
#  
#  
#  include(ICUBcontribHelpers)
#  list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
#  include(ICUBcontribHelpers)
#  include(ICUBcontribOptions)
#  icubcontrib_set_default_prefix() #set CMAKE_INSTALL_PREFIX to the ICUBcontrib one; print warning if user modifies it
#  
#  include(CheckCXXCompilerFlag)
#  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
#  CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
#  if(COMPILER_SUPPORTS_CXX11)
#      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 -O3")
#  elseif(COMPILER_SUPPORTS_CXX0X)
#      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++0x -O3")
#  else()
#      message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
#  endif()
#  
#  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#  
#  add_definitions(${YARP_DEFINES})
#  include(YarpInstallationHelpers)
#  
#  
#  find_package(OpenCV)
#  find_package(IPOPT)
#  
#  
#  #find_package(logpolar QUIET)
#  #IF (logpolar_FOUND)
#  #   message("Attention System: logpolar library detected and used.")
#  #   message ("logpolar_INCLUDE_DIRS : ${logpolar_INCLUDE_DIRS};")
#  #ELSE ()
#  #   message("Attention System: logpolar library undetected")
#  #ENDIF ()
#  
#  
#  SET(CUDAVISION_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/cudaVision/include)
#  SET(AUDIOLIB_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}/audioLib/include)
#  
#  # default prefix for applications
#  set(ICUB_APPLICATIONS_PREFIX "$ENV{ICUB_ROOT}" CACHE PATH "Application path prefix")
#  
#  ### Shall the system look for CUDA 
#  option(ENABLE_COMPILEONPC104  "Are you compiling on PC104?" OFF)
#  if (ENABLE_CUDA)  
#      find_package(CUDA QUIET REQUIRED)
#      add_definitions (-DWITH_CUDA)
#      add_subdirectory(cudaVision)
#  endif (ENABLE_CUDA)
#  
#  ### Shall the system look for CUDA 
#  option(ENABLE_MATLAB  "Do you want to link against MATLAB?" OFF)
#  #if (ENABLE_MATLAB)  
#  #    find_package(CUDA QUIET REQUIRED)
#  #    add_definitions (-DWITH_CUDA)
#  #    add_subdirectory(cudaVision)
#  #endif (ENABLE_CUDA)
#  
#  
#  # Go through single folder
#  add_subdirectory(src)
#  add_subdirectory(app)
#  add_subdirectory(audioLib)
#  add_subdirectory(Dependencies)
#  
#  icubcontrib_finalize_export(audioLib)
#  icubcontrib_finalize_export(iCubAudioAttention)
#  icubcontrib_add_uninstall_target()        #macro added 13/11/2013
#  