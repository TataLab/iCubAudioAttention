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
include src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/depend.make

# Include the progress variables for this target.
include src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/progress.make

# Include the compile flags for this target's objects.
include src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/flags.make

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/flags.make
src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o: ../src/audioMemoryMapper/src/audioMemoryMapperModule.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/austin/research/iCubAudioAttention/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o"
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o -c /home/austin/research/iCubAudioAttention/src/audioMemoryMapper/src/audioMemoryMapperModule.cpp

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.i"
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/austin/research/iCubAudioAttention/src/audioMemoryMapper/src/audioMemoryMapperModule.cpp > CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.i

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.s"
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/austin/research/iCubAudioAttention/src/audioMemoryMapper/src/audioMemoryMapperModule.cpp -o CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.s

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o.requires:

.PHONY : src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o.requires

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o.provides: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o.requires
	$(MAKE) -f src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/build.make src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o.provides.build
.PHONY : src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o.provides

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o.provides.build: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o


src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/flags.make
src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o: ../src/audioMemoryMapper/src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/austin/research/iCubAudioAttention/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o"
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o -c /home/austin/research/iCubAudioAttention/src/audioMemoryMapper/src/main.cpp

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/audioMemoryMapper.dir/src/main.cpp.i"
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/austin/research/iCubAudioAttention/src/audioMemoryMapper/src/main.cpp > CMakeFiles/audioMemoryMapper.dir/src/main.cpp.i

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/audioMemoryMapper.dir/src/main.cpp.s"
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/austin/research/iCubAudioAttention/src/audioMemoryMapper/src/main.cpp -o CMakeFiles/audioMemoryMapper.dir/src/main.cpp.s

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o.requires:

.PHONY : src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o.requires

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o.provides: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o.requires
	$(MAKE) -f src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/build.make src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o.provides.build
.PHONY : src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o.provides

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o.provides.build: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o


src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/flags.make
src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o: ../src/audioMemoryMapper/src/audioMemoryMapperRateThread.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/austin/research/iCubAudioAttention/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o"
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o -c /home/austin/research/iCubAudioAttention/src/audioMemoryMapper/src/audioMemoryMapperRateThread.cpp

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.i"
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/austin/research/iCubAudioAttention/src/audioMemoryMapper/src/audioMemoryMapperRateThread.cpp > CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.i

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.s"
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/austin/research/iCubAudioAttention/src/audioMemoryMapper/src/audioMemoryMapperRateThread.cpp -o CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.s

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o.requires:

.PHONY : src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o.requires

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o.provides: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o.requires
	$(MAKE) -f src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/build.make src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o.provides.build
.PHONY : src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o.provides

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o.provides.build: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o


# Object files for target audioMemoryMapper
audioMemoryMapper_OBJECTS = \
"CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o" \
"CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o" \
"CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o"

# External object files for target audioMemoryMapper
audioMemoryMapper_EXTERNAL_OBJECTS =

bin/audioMemoryMapper: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o
bin/audioMemoryMapper: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o
bin/audioMemoryMapper: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o
bin/audioMemoryMapper: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/build.make
bin/audioMemoryMapper: /usr/local/lib/libYARP_dev.so.2.3.68
bin/audioMemoryMapper: /usr/local/lib/libYARP_name.so.2.3.68
bin/audioMemoryMapper: /usr/local/lib/libYARP_init.so.2.3.68
bin/audioMemoryMapper: /usr/local/lib/libYARP_math.so.2.3.68
bin/audioMemoryMapper: /usr/local/lib/libYARP_sig.so.2.3.68
bin/audioMemoryMapper: /usr/local/lib/libYARP_OS.so.2.3.68
bin/audioMemoryMapper: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/austin/research/iCubAudioAttention/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable ../../bin/audioMemoryMapper"
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/audioMemoryMapper.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/build: bin/audioMemoryMapper

.PHONY : src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/build

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/requires: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperModule.cpp.o.requires
src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/requires: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/main.cpp.o.requires
src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/requires: src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/src/audioMemoryMapperRateThread.cpp.o.requires

.PHONY : src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/requires

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/clean:
	cd /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper && $(CMAKE_COMMAND) -P CMakeFiles/audioMemoryMapper.dir/cmake_clean.cmake
.PHONY : src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/clean

src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/depend:
	cd /home/austin/research/iCubAudioAttention/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/austin/research/iCubAudioAttention /home/austin/research/iCubAudioAttention/src/audioMemoryMapper /home/austin/research/iCubAudioAttention/build /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper /home/austin/research/iCubAudioAttention/build/src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/audioMemoryMapper/CMakeFiles/audioMemoryMapper.dir/depend

