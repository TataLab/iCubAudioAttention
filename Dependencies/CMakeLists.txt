# Copyright: (C) 2015 RBCS Robotics Brain and Cognitive Science
# Authors: Francesco Rea
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)
if(ENABLE_COMPILEONPC104)
    #add_subdirectory ( portaudio )
else(ENABLE_COMPILEONPC104)
    #add_subdirectory ( portaudio )
    add_subdirectory ( fftw3     )
endif(ENABLE_COMPILEONPC104)
