# Authors: Marko Ilievski
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.8.9)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)


find_package(YARP REQUIRED)
find_package(ICUB REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${ICUB_MODULE_PATH})
include(YarpInstallationHelpers)

find_package(ICUBcontrib REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${YARP_MODULE_PATH}
  ${ICUB_MODULE_PATH}
  ${ICUBCONTRIB_MODULE_PATH}
  )

include(ICUBcontribHelpers)
list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
include(ICUBcontribHelpers)
include(ICUBcontribOptions)
icubcontrib_set_default_prefix()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_definitions(${YARP_DEFINES})
include(YarpInstallationHelpers)


SET(KEYWORD "yarpPreprocessing")
PROJECT(${KEYWORD})

set(ICUB_CONTRIB_DIRS $ENV{ICUB_DIR}/include)

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/include
    ${YARP_INCLUDE_DIRS}
    ${ICUB_INCLUDE_DIRS}
    ${ICUB_CONTRIB_DIRS}
)

# Search for source code.
FILE(GLOB folder_source src/*.cpp src/*.cc src/*.c ../Configuration/*.cpp ../Configuration/*.cc ../Configuration/*.c)
FILE(GLOB folder_header include/iCub/*.h ../Configuration/*.h)
SOURCE_GROUP("Source Files" FILES ${folder_source} )
SOURCE_GROUP("Header Files" FILES ${folder_header} )

# Set up the main executable.
IF (folder_source)
    ADD_EXECUTABLE(${KEYWORD}
        ${folder_source}
        ${folder_header}
    )

    TARGET_LINK_LIBRARIES(${KEYWORD}
      ${YARP_LIBRARIES}
      )

    INSTALL_TARGETS(/bin ${KEYWORD})
ELSE (folder_source)
    MESSAGE(FATAL_ERROR "No source code files found. Please add something")
ENDIF (folder_source)
