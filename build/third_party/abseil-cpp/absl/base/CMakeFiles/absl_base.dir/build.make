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
include third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/depend.make

# Include the progress variables for this target.
include third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/progress.make

# Include the compile flags for this target's objects.
include third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/flags.make

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/flags.make
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o: ../third_party/abseil-cpp/absl/base/internal/cycleclock.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/absl_base.dir/internal/cycleclock.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/cycleclock.cc

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/absl_base.dir/internal/cycleclock.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/cycleclock.cc > CMakeFiles/absl_base.dir/internal/cycleclock.cc.i

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/absl_base.dir/internal/cycleclock.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/cycleclock.cc -o CMakeFiles/absl_base.dir/internal/cycleclock.cc.s

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o.requires:

.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o.requires

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o.provides: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o.requires
	$(MAKE) -f third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/build.make third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o.provides.build
.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o.provides

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o.provides.build: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o


third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/flags.make
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o: ../third_party/abseil-cpp/absl/base/internal/raw_logging.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/absl_base.dir/internal/raw_logging.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/raw_logging.cc

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/absl_base.dir/internal/raw_logging.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/raw_logging.cc > CMakeFiles/absl_base.dir/internal/raw_logging.cc.i

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/absl_base.dir/internal/raw_logging.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/raw_logging.cc -o CMakeFiles/absl_base.dir/internal/raw_logging.cc.s

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o.requires:

.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o.requires

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o.provides: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o.requires
	$(MAKE) -f third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/build.make third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o.provides.build
.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o.provides

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o.provides.build: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o


third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/flags.make
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o: ../third_party/abseil-cpp/absl/base/internal/spinlock.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/absl_base.dir/internal/spinlock.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/spinlock.cc

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/absl_base.dir/internal/spinlock.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/spinlock.cc > CMakeFiles/absl_base.dir/internal/spinlock.cc.i

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/absl_base.dir/internal/spinlock.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/spinlock.cc -o CMakeFiles/absl_base.dir/internal/spinlock.cc.s

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o.requires:

.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o.requires

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o.provides: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o.requires
	$(MAKE) -f third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/build.make third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o.provides.build
.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o.provides

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o.provides.build: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o


third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/flags.make
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o: ../third_party/abseil-cpp/absl/base/internal/sysinfo.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/absl_base.dir/internal/sysinfo.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/sysinfo.cc

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/absl_base.dir/internal/sysinfo.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/sysinfo.cc > CMakeFiles/absl_base.dir/internal/sysinfo.cc.i

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/absl_base.dir/internal/sysinfo.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/sysinfo.cc -o CMakeFiles/absl_base.dir/internal/sysinfo.cc.s

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o.requires:

.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o.requires

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o.provides: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o.requires
	$(MAKE) -f third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/build.make third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o.provides.build
.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o.provides

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o.provides.build: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o


third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/flags.make
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o: ../third_party/abseil-cpp/absl/base/internal/thread_identity.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/absl_base.dir/internal/thread_identity.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/thread_identity.cc

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/absl_base.dir/internal/thread_identity.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/thread_identity.cc > CMakeFiles/absl_base.dir/internal/thread_identity.cc.i

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/absl_base.dir/internal/thread_identity.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/thread_identity.cc -o CMakeFiles/absl_base.dir/internal/thread_identity.cc.s

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o.requires:

.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o.requires

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o.provides: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o.requires
	$(MAKE) -f third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/build.make third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o.provides.build
.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o.provides

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o.provides.build: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o


third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/flags.make
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o: ../third_party/abseil-cpp/absl/base/internal/unscaledcycleclock.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/unscaledcycleclock.cc

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/unscaledcycleclock.cc > CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.i

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/internal/unscaledcycleclock.cc -o CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.s

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o.requires:

.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o.requires

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o.provides: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o.requires
	$(MAKE) -f third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/build.make third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o.provides.build
.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o.provides

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o.provides.build: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o


third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/flags.make
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o: ../third_party/abseil-cpp/absl/base/log_severity.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/absl_base.dir/log_severity.cc.o -c /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/log_severity.cc

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/absl_base.dir/log_severity.cc.i"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/log_severity.cc > CMakeFiles/absl_base.dir/log_severity.cc.i

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/absl_base.dir/log_severity.cc.s"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base/log_severity.cc -o CMakeFiles/absl_base.dir/log_severity.cc.s

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o.requires:

.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o.requires

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o.provides: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o.requires
	$(MAKE) -f third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/build.make third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o.provides.build
.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o.provides

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o.provides.build: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o


# Object files for target absl_base
absl_base_OBJECTS = \
"CMakeFiles/absl_base.dir/internal/cycleclock.cc.o" \
"CMakeFiles/absl_base.dir/internal/raw_logging.cc.o" \
"CMakeFiles/absl_base.dir/internal/spinlock.cc.o" \
"CMakeFiles/absl_base.dir/internal/sysinfo.cc.o" \
"CMakeFiles/absl_base.dir/internal/thread_identity.cc.o" \
"CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o" \
"CMakeFiles/absl_base.dir/log_severity.cc.o"

# External object files for target absl_base
absl_base_EXTERNAL_OBJECTS =

third_party/abseil-cpp/absl/base/libabsl_absl_base.a: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o
third_party/abseil-cpp/absl/base/libabsl_absl_base.a: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o
third_party/abseil-cpp/absl/base/libabsl_absl_base.a: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o
third_party/abseil-cpp/absl/base/libabsl_absl_base.a: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o
third_party/abseil-cpp/absl/base/libabsl_absl_base.a: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o
third_party/abseil-cpp/absl/base/libabsl_absl_base.a: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o
third_party/abseil-cpp/absl/base/libabsl_absl_base.a: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o
third_party/abseil-cpp/absl/base/libabsl_absl_base.a: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/build.make
third_party/abseil-cpp/absl/base/libabsl_absl_base.a: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/workSpace/janus-gateway/media-gateway/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Linking CXX static library libabsl_absl_base.a"
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && $(CMAKE_COMMAND) -P CMakeFiles/absl_base.dir/cmake_clean_target.cmake
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/absl_base.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/build: third_party/abseil-cpp/absl/base/libabsl_absl_base.a

.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/build

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/requires: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/cycleclock.cc.o.requires
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/requires: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/raw_logging.cc.o.requires
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/requires: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/spinlock.cc.o.requires
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/requires: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/sysinfo.cc.o.requires
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/requires: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/thread_identity.cc.o.requires
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/requires: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/internal/unscaledcycleclock.cc.o.requires
third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/requires: third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/log_severity.cc.o.requires

.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/requires

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/clean:
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base && $(CMAKE_COMMAND) -P CMakeFiles/absl_base.dir/cmake_clean.cmake
.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/clean

third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/depend:
	cd /home/ubuntu/workSpace/janus-gateway/media-gateway/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/workSpace/janus-gateway/media-gateway /home/ubuntu/workSpace/janus-gateway/media-gateway/third_party/abseil-cpp/absl/base /home/ubuntu/workSpace/janus-gateway/media-gateway/build /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base /home/ubuntu/workSpace/janus-gateway/media-gateway/build/third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : third_party/abseil-cpp/absl/base/CMakeFiles/absl_base.dir/depend

