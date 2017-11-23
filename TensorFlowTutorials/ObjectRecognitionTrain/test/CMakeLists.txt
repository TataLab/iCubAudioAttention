# Copyright (C) 2017 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
# Authors: Francesco Rea & Gonzalez Jonas
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

cmake_minimum_required(VERSION 2.6)
SET(KEYWORD "CameraStream")
PROJECT(${KEYWORD})
set(CMAKE_CXX_STANDARD 11)


FIND_PACKAGE(YARP REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)
find_package(ICUB REQUIRED)

#search for includes
INCLUDE_DIRECTORIES(
        ${PROJECT_SOURCE_DIR}/include
        ${YARP_INCLUDE_DIRS}
        ${OpenCV_DIRS}
)


ADD_EXECUTABLE(${KEYWORD}
         main.cpp
         )

TARGET_LINK_LIBRARIES( ${KEYWORD}
         ${YARP_LIBRARIES}
         ${OpenCV_LIBS}
         )

INSTALL_TARGETS(/bin ${KEYWORD})