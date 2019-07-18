# Copyright: (C) 2019 Department of Neuroscience - University of Lethbridge
# Authors: Austin Kothig <kothiga@uleth.ca>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 3.5)

set(MODULE_NAME audioPreprocessor)
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
        PRIVATE ${PROJECT_SOURCE_DIR}/include ${fftw_SOURCE_DIR}/api
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
