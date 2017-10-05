# Copyright: (C) 2010 RobotCub Consortium
# Authors: Francesco Rea, Austin Kothig, Marko Ilievski
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

#cmake_minimum_required(VERSION 2.6)

#add_subdirectory ( audioVisualAttention )
add_subdirectory ( icubAudioAttention		)
add_subdirectory ( memoryMapper			)
add_subdirectory ( frequencyVisualisation	)
add_subdirectory ( robotStartUp			)

## make all applications show up as single target
#icub_app_all()
