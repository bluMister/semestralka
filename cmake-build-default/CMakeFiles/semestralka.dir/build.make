# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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
CMAKE_SOURCE_DIR = /tmp/tmp.yesXpqu0WK

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /tmp/tmp.yesXpqu0WK/cmake-build-default

# Include any dependencies generated for this target.
include CMakeFiles/semestralka.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/semestralka.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/semestralka.dir/flags.make

CMakeFiles/semestralka.dir/main.c.o: CMakeFiles/semestralka.dir/flags.make
CMakeFiles/semestralka.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/tmp/tmp.yesXpqu0WK/cmake-build-default/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/semestralka.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/semestralka.dir/main.c.o -c /tmp/tmp.yesXpqu0WK/main.c

CMakeFiles/semestralka.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/semestralka.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /tmp/tmp.yesXpqu0WK/main.c > CMakeFiles/semestralka.dir/main.c.i

CMakeFiles/semestralka.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/semestralka.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /tmp/tmp.yesXpqu0WK/main.c -o CMakeFiles/semestralka.dir/main.c.s

# Object files for target semestralka
semestralka_OBJECTS = \
"CMakeFiles/semestralka.dir/main.c.o"

# External object files for target semestralka
semestralka_EXTERNAL_OBJECTS =

semestralka: CMakeFiles/semestralka.dir/main.c.o
semestralka: CMakeFiles/semestralka.dir/build.make
semestralka: CMakeFiles/semestralka.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/tmp/tmp.yesXpqu0WK/cmake-build-default/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable semestralka"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/semestralka.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/semestralka.dir/build: semestralka

.PHONY : CMakeFiles/semestralka.dir/build

CMakeFiles/semestralka.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/semestralka.dir/cmake_clean.cmake
.PHONY : CMakeFiles/semestralka.dir/clean

CMakeFiles/semestralka.dir/depend:
	cd /tmp/tmp.yesXpqu0WK/cmake-build-default && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /tmp/tmp.yesXpqu0WK /tmp/tmp.yesXpqu0WK /tmp/tmp.yesXpqu0WK/cmake-build-default /tmp/tmp.yesXpqu0WK/cmake-build-default /tmp/tmp.yesXpqu0WK/cmake-build-default/CMakeFiles/semestralka.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/semestralka.dir/depend

