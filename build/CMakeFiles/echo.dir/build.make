# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/guagua/Desktop/work/network

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/guagua/Desktop/work/network/build

# Include any dependencies generated for this target.
include CMakeFiles/echo.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/echo.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/echo.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/echo.dir/flags.make

CMakeFiles/echo.dir/EchoServer.cc.o: CMakeFiles/echo.dir/flags.make
CMakeFiles/echo.dir/EchoServer.cc.o: ../EchoServer.cc
CMakeFiles/echo.dir/EchoServer.cc.o: CMakeFiles/echo.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/guagua/Desktop/work/network/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/echo.dir/EchoServer.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/echo.dir/EchoServer.cc.o -MF CMakeFiles/echo.dir/EchoServer.cc.o.d -o CMakeFiles/echo.dir/EchoServer.cc.o -c /home/guagua/Desktop/work/network/EchoServer.cc

CMakeFiles/echo.dir/EchoServer.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/echo.dir/EchoServer.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/guagua/Desktop/work/network/EchoServer.cc > CMakeFiles/echo.dir/EchoServer.cc.i

CMakeFiles/echo.dir/EchoServer.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/echo.dir/EchoServer.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/guagua/Desktop/work/network/EchoServer.cc -o CMakeFiles/echo.dir/EchoServer.cc.s

# Object files for target echo
echo_OBJECTS = \
"CMakeFiles/echo.dir/EchoServer.cc.o"

# External object files for target echo
echo_EXTERNAL_OBJECTS =

../bin/echo: CMakeFiles/echo.dir/EchoServer.cc.o
../bin/echo: CMakeFiles/echo.dir/build.make
../bin/echo: ../lib/libmynet.so
../bin/echo: CMakeFiles/echo.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/guagua/Desktop/work/network/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/echo"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/echo.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/echo.dir/build: ../bin/echo
.PHONY : CMakeFiles/echo.dir/build

CMakeFiles/echo.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/echo.dir/cmake_clean.cmake
.PHONY : CMakeFiles/echo.dir/clean

CMakeFiles/echo.dir/depend:
	cd /home/guagua/Desktop/work/network/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/guagua/Desktop/work/network /home/guagua/Desktop/work/network /home/guagua/Desktop/work/network/build /home/guagua/Desktop/work/network/build /home/guagua/Desktop/work/network/build/CMakeFiles/echo.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/echo.dir/depend

