# Install script for directory: /Users/Matthew/Documents/Robotics/iCubAudioAttention/EventDrivenAudioAttention/AudioCapture/audioRemoteInit

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

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/mex" TYPE SHARED_LIBRARY FILES "/Users/Matthew/Documents/Robotics/iCubAudioAttention/EventDrivenAudioAttention/AudioCapture/audioRemoteInit/build/wholeBodyModel.mexmaci64")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/mex/wholeBodyModel.mexmaci64" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/mex/wholeBodyModel.mexmaci64")
    execute_process(COMMAND "/usr/bin/install_name_tool"
      -id "wholeBodyModel.mexmaci64"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/mex/wholeBodyModel.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/Matthew/Documents/Robotics/iCubAudioAttention/EventDrivenAudioAttention/AudioCapture/audioRemoteInit/build"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/mex/wholeBodyModel.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Applications/MATLAB_R2014a.app/bin/maci64"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/mex/wholeBodyModel.mexmaci64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/mex/wholeBodyModel.mexmaci64")
    endif()
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/Matthew/Documents/Robotics/iCubAudioAttention/EventDrivenAudioAttention/AudioCapture/audioRemoteInit/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
