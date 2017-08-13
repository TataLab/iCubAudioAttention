# Install script for directory: /home/austin/research/iCubAudioAttention/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/austin/research/iCubAudioAttention/build/src/egoNoiseCalibration/cmake_install.cmake")
  include("/home/austin/research/iCubAudioAttention/build/src/mapAudio/cmake_install.cmake")
  include("/home/austin/research/iCubAudioAttention/build/src/headAngleMemoryMapper/cmake_install.cmake")
  include("/home/austin/research/iCubAudioAttention/build/src/audioPreprocessing/cmake_install.cmake")
  include("/home/austin/research/iCubAudioAttention/build/src/audioBayesianMap/cmake_install.cmake")
  include("/home/austin/research/iCubAudioAttention/build/src/frequencyVisualisation/cmake_install.cmake")
  include("/home/austin/research/iCubAudioAttention/build/src/soundMonitor/cmake_install.cmake")
  include("/home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper/cmake_install.cmake")

endif()

