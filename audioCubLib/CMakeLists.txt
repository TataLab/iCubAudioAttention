# Copyright: (C) 2019 Department of Neuroscience - University of Lethbridge
# Authors: Austin Kothig <kothiga@uleth.ca>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 3.5)

set(LIB_NAME audioCub)
project(${LIB_NAME} LANGUAGES CXX C VERSION 1.0.0)

############################################################

option(ENABLE_OMP "Do you want to link against OpenMP?" OFF)
if( ENABLE_OMP )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
    add_definitions( -DWITH_OMP )

endif( ENABLE_OMP )

############################################################

set(${PROJECT_NAME}_SRC
    src/audioUtil.cpp
    #src/Sound.cpp
    #src/SoundFile.cpp
    #src/spatialSound.cpp
)

set(${PROJECT_NAME}_HDR
    include/iCub/util/audioUtil.h
    #include/iCub/audio/Sound.h
    #include/iCub/audio/SoundFile.h
    #include/iCub/audio/spatialSound.h
)

if( ENABLE_COMPILE_ON_PC104 )
    
    # Don't compile parts of the lib the pc104 doesn't need.

else( ENABLE_COMPILE_ON_PC104 )

    list(APPEND ${PROJECT_NAME}_SRC
        src/beamformer.cpp
        src/butterworthFilter.cpp
        src/gammatoneFilterBank.cpp
        src/hilbertTransform.cpp
    )
    
    list(APPEND ${PROJECT_NAME}_HDR
        include/iCub/filters/beamformer.h
        include/iCub/filters/butterworthFilter.h
        include/iCub/filters/gammatoneFilterBank.h
        include/iCub/filters/hilbertTransform.h
    )

endif( ENABLE_COMPILE_ON_PC104 )

############################################################

add_library(
    ${PROJECT_NAME}
    ${${PROJECT_NAME}_SRC} 
    ${${PROJECT_NAME}_HDR}
)

set_target_properties(
    ${PROJECT_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION}
    PUBLIC_HEADER "${${PROJECT_NAME}_HDR}"
)

target_link_libraries(
    ${PROJECT_NAME}   
    ${YARP_LIBRARIES} 
    fftw3
)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${PROJECT_SOURCE_DIR}/include ${fftw_SOURCE_DIR}/api
    PUBLIC  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>"    
)

############################################################

install(
    TARGETS       ${PROJECT_NAME}
    EXPORT        ${PROJECT_NAME}
    LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                     COMPONENT shlib
    ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                     COMPONENT lib
    RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                     COMPONENT bin
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}" COMPONENT dev
)

include(InstallBasicPackageFiles)
install_basic_package_files(
    ${PROJECT_NAME} 
    VERSION         ${${PROJECT_NAME}_VERSION} 
    COMPATIBILITY   AnyNewerVersion
    EXPORT          ${PROJECT_NAME}
    VARS_PREFIX     ${PROJECT_NAME}
    DEPENDENCIES    "YARP REQUIRED"
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

############################################################
