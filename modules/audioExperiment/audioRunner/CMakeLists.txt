# Copyright: (C) 2019 Department of Neuroscience - University of Lethbridge
# Authors: Austin Kothig <kothiga@uleth.ca>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 3.5)

set(MODULE_NAME audioRunner)
project(${MODULE_NAME} LANGUAGES CXX C)

############################################################

if( ENABLE_OMP )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
    add_definitions( -DWITH_OMP )

endif( ENABLE_OMP )

############################################################

file(GLOB folder_source src/*.cpp src/*.cc src/*.c)
file(GLOB folder_header include/iCub/*.h)

source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_header})

############################################################

if( folder_source )

    add_executable(
        ${PROJECT_NAME}
        ${folder_header}
        ${folder_source}
    )
    
    target_include_directories(
        ${PROJECT_NAME}
        PRIVATE ${PROJECT_SOURCE_DIR}/include
    )
    
    target_link_libraries(
        ${PROJECT_NAME}
        audioCub
        ${YARP_LIBRARIES}
    )
    
    install(
        TARGETS
        ${PROJECT_NAME}
        DESTINATION bin    
    )

else( folder_source )
    
    message(FATAL_ERROR "No source code files found. Please add something.")

endif( folder_source )

############################################################





# # Copyright: (C) 2018 Department of Neuroscience - University of Lethbridge
# # Authors: Austin Kothig
# # CopyPolicy: Released under the terms of the GNU GPL v2.0.
# 
# cmake_minimum_required(VERSION 2.8.9)
# 
# if(COMMAND cmake_policy)
#     cmake_policy(SET CMP0003 NEW)
# endif(COMMAND cmake_policy)
# 
# SET(KEYWORD "audioRunner")
# PROJECT(${KEYWORD})
# 
# find_package(YARP REQUIRED)
# find_package(ICUB REQUIRED)
# 
# list(APPEND CMAKE_MODULE_PATH ${YARP_MODULE_PATH})
# list(APPEND CMAKE_MODULE_PATH ${ICUB_MODULE_PATH})
# include(YarpInstallationHelpers)
# 
# find_package(ICUBcontrib REQUIRED)
# list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
# 
# set(CMAKE_MODULE_PATH
#   ${CMAKE_MODULE_PATH}
#   ${YARP_MODULE_PATH}
#   ${ICUB_MODULE_PATH}
#   ${ICUBCONTRIB_MODULE_PATH}
# )
# 
# include(ICUBcontribHelpers)
# list(APPEND CMAKE_MODULE_PATH ${ICUBCONTRIB_MODULE_PATH})
# include(ICUBcontribHelpers)
# include(ICUBcontribOptions)
# icubcontrib_set_default_prefix()
# 
# list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# 
# add_definitions(${YARP_DEFINES})
# include(YarpInstallationHelpers)
# 
# INCLUDE_DIRECTORIES(
#     ${PROJECT_SOURCE_DIR}/include
#     ${YARP_INCLUDE_DIRS} 	
#     ${ICUB_INCLUDE_DIRS}	
#     ${ICUB_CONTRIB_DIRS}
#     ${audioLib_INCLUDE_DIRS}
# )
# 
# # Search for source code.
# FILE(GLOB folder_source src/*.cpp src/*.cc src/*.c)
# FILE(GLOB folder_header include/iCub/*.h)
# SOURCE_GROUP("Source Files" FILES ${folder_source})
# SOURCE_GROUP("Header Files" FILES ${folder_header})
# 
# # Set up the main executable.
# IF (folder_source)
#     ADD_EXECUTABLE(${KEYWORD} 
#         ${folder_source} 
#         ${folder_header}
#     )
# 
#     TARGET_LINK_LIBRARIES(${KEYWORD}  
#       audioLib      
#       ${YARP_LIBRARIES}
#       )	
# 
#     INSTALL_TARGETS(/bin ${KEYWORD})
# ELSE (folder_source)
#     MESSAGE(FATAL_ERROR "No source code files found. Please add something")
# ENDIF (folder_source)
# 