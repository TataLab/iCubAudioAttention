# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/austin/research/iCubAudioAttention

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/austin/research/iCubAudioAttention/build

# Include any dependencies generated for this target.
include src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/depend.make

# Include the progress variables for this target.
include src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/progress.make

# Include the compile flags for this target's objects.
include src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/flags.make

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/flags.make
src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o: ../src/YarpBayesianMap/src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/austin/research/iCubAudioAttention/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o -c /home/austin/research/iCubAudioAttention/src/YarpBayesianMap/src/main.cpp

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/yarpBayesianMap.dir/src/main.cpp.i"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/austin/research/iCubAudioAttention/src/YarpBayesianMap/src/main.cpp > CMakeFiles/yarpBayesianMap.dir/src/main.cpp.i

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/yarpBayesianMap.dir/src/main.cpp.s"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/austin/research/iCubAudioAttention/src/YarpBayesianMap/src/main.cpp -o CMakeFiles/yarpBayesianMap.dir/src/main.cpp.s

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o.requires:

.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o.requires

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o.provides: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o.requires
	$(MAKE) -f src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/build.make src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o.provides.build
.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o.provides

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o.provides.build: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o


src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/flags.make
src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o: ../src/YarpBayesianMap/src/BayesianModule.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/austin/research/iCubAudioAttention/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o -c /home/austin/research/iCubAudioAttention/src/YarpBayesianMap/src/BayesianModule.cc

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.i"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/austin/research/iCubAudioAttention/src/YarpBayesianMap/src/BayesianModule.cc > CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.i

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.s"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/austin/research/iCubAudioAttention/src/YarpBayesianMap/src/BayesianModule.cc -o CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.s

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o.requires:

.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o.requires

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o.provides: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o.requires
	$(MAKE) -f src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/build.make src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o.provides.build
.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o.provides

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o.provides.build: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o


src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/flags.make
src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o: ../src/Configuration/Config.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/austin/research/iCubAudioAttention/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o -c /home/austin/research/iCubAudioAttention/src/Configuration/Config.cpp

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.i"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/austin/research/iCubAudioAttention/src/Configuration/Config.cpp > CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.i

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.s"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/austin/research/iCubAudioAttention/src/Configuration/Config.cpp -o CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.s

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o.requires:

.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o.requires

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o.provides: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o.requires
	$(MAKE) -f src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/build.make src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o.provides.build
.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o.provides

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o.provides.build: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o


src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/flags.make
src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o: ../src/Configuration/ConfigParser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/austin/research/iCubAudioAttention/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o -c /home/austin/research/iCubAudioAttention/src/Configuration/ConfigParser.cpp

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.i"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/austin/research/iCubAudioAttention/src/Configuration/ConfigParser.cpp > CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.i

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.s"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/austin/research/iCubAudioAttention/src/Configuration/ConfigParser.cpp -o CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.s

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o.requires:

.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o.requires

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o.provides: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o.requires
	$(MAKE) -f src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/build.make src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o.provides.build
.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o.provides

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o.provides.build: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o


# Object files for target yarpBayesianMap
yarpBayesianMap_OBJECTS = \
"CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o" \
"CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o" \
"CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o" \
"CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o"

# External object files for target yarpBayesianMap
yarpBayesianMap_EXTERNAL_OBJECTS =

bin/yarpBayesianMap: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o
bin/yarpBayesianMap: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o
bin/yarpBayesianMap: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o
bin/yarpBayesianMap: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o
bin/yarpBayesianMap: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/build.make
bin/yarpBayesianMap: /usr/local/lib/libYARP_dev.so.2.3.68
bin/yarpBayesianMap: /usr/local/lib/libYARP_name.so.2.3.68
bin/yarpBayesianMap: /usr/local/lib/libYARP_init.so.2.3.68
bin/yarpBayesianMap: /usr/local/lib/libYARP_math.so.2.3.68
bin/yarpBayesianMap: /usr/local/lib/libYARP_sig.so.2.3.68
bin/yarpBayesianMap: /usr/local/lib/libYARP_OS.so.2.3.68
bin/yarpBayesianMap: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/austin/research/iCubAudioAttention/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable ../../bin/yarpBayesianMap"
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/yarpBayesianMap.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/build: bin/yarpBayesianMap

.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/build

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/requires: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/main.cpp.o.requires
src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/requires: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/src/BayesianModule.cc.o.requires
src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/requires: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/Config.cpp.o.requires
src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/requires: src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/__/Configuration/ConfigParser.cpp.o.requires

.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/requires

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/clean:
	cd /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap && $(CMAKE_COMMAND) -P CMakeFiles/yarpBayesianMap.dir/cmake_clean.cmake
.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/clean

src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/depend:
	cd /home/austin/research/iCubAudioAttention/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/austin/research/iCubAudioAttention /home/austin/research/iCubAudioAttention/src/YarpBayesianMap /home/austin/research/iCubAudioAttention/build /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap /home/austin/research/iCubAudioAttention/build/src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/YarpBayesianMap/CMakeFiles/yarpBayesianMap.dir/depend

