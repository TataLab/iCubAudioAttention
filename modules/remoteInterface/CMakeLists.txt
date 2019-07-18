# Copyright: (C) 2019 Department of Neuroscience - University of Lethbridge
# Authors: Austin Kothig <kothiga@uleth.ca>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 3.5)

set(MODULE_NAME remoteInterface)
project(${MODULE_NAME} LANGUAGES CXX C)

############################################################

file(GLOB folder_source *.cpp *.cc *.c)

source_group("Source Files" FILES ${folder_source})

############################################################

if( folder_source )

    add_executable(
        ${PROJECT_NAME}
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
