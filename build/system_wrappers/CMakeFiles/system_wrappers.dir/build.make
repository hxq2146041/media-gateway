# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_SOURCE_DIR = /home/ubuntu/workSpace/janus-gateway/media-gateway

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/workSpace/janus-gateway/media-gateway/build

# Include any dependencies generated for this target.
include system_wrappers/CMakeFiles/system_wrappers.dir/depend.make

# Include the progress variables for this target.
include system_wrappers/CMakeFiles/system_wrappers.dir/progress.make

# Include the compile flags for this target's objects.
include system_wrappers/CMakeFiles/system_wrappers.dir/flags.make

system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o: system_wrappers/CMakeFiles/system_wrappers.dir/flags.make
system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o: ../system_wrappers/source/clock.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/system_wrappers.dir/source/clock.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/clock.cc

system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/system_wrappers.dir/source/clock.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/clock.cc > CMakeFiles/system_wrappers.dir/source/clock.cc.i

system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/system_wrappers.dir/source/clock.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/clock.cc -o CMakeFiles/system_wrappers.dir/source/clock.cc.s

system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o.requires:

.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o.requires

system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o.provides: system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o.requires
	$(MAKE) -f system_wrappers/CMakeFiles/system_wrappers.dir/build.make system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o.provides.build
.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o.provides

system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o.provides.build: system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o


system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o: system_wrappers/CMakeFiles/system_wrappers.dir/flags.make
system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o: ../system_wrappers/source/cpu_features.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/cpu_features.cc

system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/system_wrappers.dir/source/cpu_features.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/cpu_features.cc > CMakeFiles/system_wrappers.dir/source/cpu_features.cc.i

system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/system_wrappers.dir/source/cpu_features.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/cpu_features.cc -o CMakeFiles/system_wrappers.dir/source/cpu_features.cc.s

system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o.requires:

.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o.requires

system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o.provides: system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o.requires
	$(MAKE) -f system_wrappers/CMakeFiles/system_wrappers.dir/build.make system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o.provides.build
.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o.provides

system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o.provides.build: system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o


system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o: system_wrappers/CMakeFiles/system_wrappers.dir/flags.make
system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o: ../system_wrappers/source/cpu_info.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/cpu_info.cc

system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/system_wrappers.dir/source/cpu_info.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/cpu_info.cc > CMakeFiles/system_wrappers.dir/source/cpu_info.cc.i

system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/system_wrappers.dir/source/cpu_info.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/cpu_info.cc -o CMakeFiles/system_wrappers.dir/source/cpu_info.cc.s

system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o.requires:

.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o.requires

system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o.provides: system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o.requires
	$(MAKE) -f system_wrappers/CMakeFiles/system_wrappers.dir/build.make system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o.provides.build
.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o.provides

system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o.provides.build: system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o


system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o: system_wrappers/CMakeFiles/system_wrappers.dir/flags.make
system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o: ../system_wrappers/source/sleep.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/system_wrappers.dir/source/sleep.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/sleep.cc

system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/system_wrappers.dir/source/sleep.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/sleep.cc > CMakeFiles/system_wrappers.dir/source/sleep.cc.i

system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/system_wrappers.dir/source/sleep.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers/source/sleep.cc -o CMakeFiles/system_wrappers.dir/source/sleep.cc.s

system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o.requires:

.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o.requires

system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o.provides: system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o.requires
	$(MAKE) -f system_wrappers/CMakeFiles/system_wrappers.dir/build.make system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o.provides.build
.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o.provides

system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o.provides.build: system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o


# Object files for target system_wrappers
system_wrappers_OBJECTS = \
"CMakeFiles/system_wrappers.dir/source/clock.cc.o" \
"CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o" \
"CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o" \
"CMakeFiles/system_wrappers.dir/source/sleep.cc.o"

# External object files for target system_wrappers
system_wrappers_EXTERNAL_OBJECTS =

system_wrappers/libsystem_wrappers.a: system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o
system_wrappers/libsystem_wrappers.a: system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o
system_wrappers/libsystem_wrappers.a: system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o
system_wrappers/libsystem_wrappers.a: system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o
system_wrappers/libsystem_wrappers.a: system_wrappers/CMakeFiles/system_wrappers.dir/build.make
system_wrappers/libsystem_wrappers.a: system_wrappers/CMakeFiles/system_wrappers.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX static library libsystem_wrappers.a"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && $(CMAKE_COMMAND) -P CMakeFiles/system_wrappers.dir/cmake_clean_target.cmake
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/system_wrappers.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
system_wrappers/CMakeFiles/system_wrappers.dir/build: system_wrappers/libsystem_wrappers.a

.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/build

system_wrappers/CMakeFiles/system_wrappers.dir/requires: system_wrappers/CMakeFiles/system_wrappers.dir/source/clock.cc.o.requires
system_wrappers/CMakeFiles/system_wrappers.dir/requires: system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_features.cc.o.requires
system_wrappers/CMakeFiles/system_wrappers.dir/requires: system_wrappers/CMakeFiles/system_wrappers.dir/source/cpu_info.cc.o.requires
system_wrappers/CMakeFiles/system_wrappers.dir/requires: system_wrappers/CMakeFiles/system_wrappers.dir/source/sleep.cc.o.requires

.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/requires

system_wrappers/CMakeFiles/system_wrappers.dir/clean:
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers && $(CMAKE_COMMAND) -P CMakeFiles/system_wrappers.dir/cmake_clean.cmake
.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/clean

system_wrappers/CMakeFiles/system_wrappers.dir/depend:
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/workSpace/janus-gateway/media-gateway /home/ubuntu/workSpace/janus-gateway/media-gateway/system_wrappers /home/ubuntu/workSpace/janus-gateway/media-gateway/build /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers /home/ubuntu/workSpace/janus-gateway/media-gateway/build/system_wrappers/CMakeFiles/system_wrappers.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : system_wrappers/CMakeFiles/system_wrappers.dir/depend

