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
CMAKE_SOURCE_DIR = /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64

# Include any dependencies generated for this target.
include exrstdattr/CMakeFiles/exrstdattr.dir/depend.make

# Include the progress variables for this target.
include exrstdattr/CMakeFiles/exrstdattr.dir/progress.make

# Include the compile flags for this target's objects.
include exrstdattr/CMakeFiles/exrstdattr.dir/flags.make

exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o: exrstdattr/CMakeFiles/exrstdattr.dir/flags.make
exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o: ../exrstdattr/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o"
	cd /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrstdattr && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/exrstdattr.dir/main.cpp.o -c /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/exrstdattr/main.cpp

exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/exrstdattr.dir/main.cpp.i"
	cd /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrstdattr && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/exrstdattr/main.cpp > CMakeFiles/exrstdattr.dir/main.cpp.i

exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/exrstdattr.dir/main.cpp.s"
	cd /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrstdattr && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/exrstdattr/main.cpp -o CMakeFiles/exrstdattr.dir/main.cpp.s

exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o.requires:

.PHONY : exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o.requires

exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o.provides: exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o.requires
	$(MAKE) -f exrstdattr/CMakeFiles/exrstdattr.dir/build.make exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o.provides.build
.PHONY : exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o.provides

exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o.provides.build: exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o


# Object files for target exrstdattr
exrstdattr_OBJECTS = \
"CMakeFiles/exrstdattr.dir/main.cpp.o"

# External object files for target exrstdattr
exrstdattr_EXTERNAL_OBJECTS =

exrstdattr/exrstdattr: exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o
exrstdattr/exrstdattr: exrstdattr/CMakeFiles/exrstdattr.dir/build.make
exrstdattr/exrstdattr: IlmImf/libIlmImf-2_2.so.22.0.0
exrstdattr/exrstdattr: /usr/lib/x86_64-linux-gnu/libz.so
exrstdattr/exrstdattr: /usr/lib/x86_64-linux-gnu/libz.so
exrstdattr/exrstdattr: exrstdattr/CMakeFiles/exrstdattr.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable exrstdattr"
	cd /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrstdattr && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/exrstdattr.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
exrstdattr/CMakeFiles/exrstdattr.dir/build: exrstdattr/exrstdattr

.PHONY : exrstdattr/CMakeFiles/exrstdattr.dir/build

exrstdattr/CMakeFiles/exrstdattr.dir/requires: exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.o.requires

.PHONY : exrstdattr/CMakeFiles/exrstdattr.dir/requires

exrstdattr/CMakeFiles/exrstdattr.dir/clean:
	cd /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrstdattr && $(CMAKE_COMMAND) -P CMakeFiles/exrstdattr.dir/cmake_clean.cmake
.PHONY : exrstdattr/CMakeFiles/exrstdattr.dir/clean

exrstdattr/CMakeFiles/exrstdattr.dir/depend:
	cd /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/exrstdattr /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64 /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrstdattr /media/gga/Datos/code/applications/mrViewer/dependencies/OpenEXR/build-linux64/exrstdattr/CMakeFiles/exrstdattr.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : exrstdattr/CMakeFiles/exrstdattr.dir/depend

