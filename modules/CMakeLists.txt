# Copyright: (C) 2019 Department of Neuroscience - University of Lethbridge
# Authors: Austin Kothig <kothiga@uleth.ca>
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 3.5)

if( ENABLE_COMPILE_ON_PC104 )

    add_subdirectory( remoteInterface )

else( ENABLE_COMPILE_ON_PC104 )
    
    add_subdirectory( audioPreprocessor )

endif( ENABLE_COMPILE_ON_PC104 )
