# Copyright: (C) 2015 RBCS Robotics Brain and Cognitive Science
# Authors: Francesco Rea
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)
#add_subdirectory   ( headingAudio             )
add_subdirectory    ( remoteInterface          )
add_subdirectory    ( mosaicPanHead            )
if(ENABLE_MATLAB)
  add_subdirectory  ( audioCapture             )
  add_subdirectory  ( audioAttentionControl    )
  add_subdirectory  ( audioAttentionFeedback   )
endif(ENABLE_MATLAB)
