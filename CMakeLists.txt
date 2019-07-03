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

if( MSVC )
    set(CMAKE_DEBUG_POSTFIX "d")
endif( )

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

if( NOT CMAKE_CONFIGURATION_TYPES )
    if( NOT CMAKE_BUILD_TYPE )
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif( )
endif( )

############################################################

icubcontrib_set_default_prefix()

option(ENABLE_COMPILE_ON_PC104  "Are you compiling on PC104?" OFF)

add_subdirectory( audioCubLib )
include(AddUninstallTarget)

add_subdirectory( dependencies )
add_subdirectory( modules      )
add_subdirectory( app          )

############################################################
