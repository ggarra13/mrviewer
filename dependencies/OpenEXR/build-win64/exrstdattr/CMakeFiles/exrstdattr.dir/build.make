# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = D:\code\applications\mrViewer\dependencies\OpenEXR

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64

# Include any dependencies generated for this target.
include exrstdattr\CMakeFiles\exrstdattr.dir\depend.make

# Include the progress variables for this target.
include exrstdattr\CMakeFiles\exrstdattr.dir\progress.make

# Include the compile flags for this target's objects.
include exrstdattr\CMakeFiles\exrstdattr.dir\flags.make

exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj: exrstdattr\CMakeFiles\exrstdattr.dir\flags.make
exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj: ..\exrstdattr\main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object exrstdattr/CMakeFiles/exrstdattr.dir/main.cpp.obj"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64\exrstdattr
	C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoCMakeFiles\exrstdattr.dir\main.cpp.obj /FdCMakeFiles\exrstdattr.dir\ /FS -c D:\code\applications\mrViewer\dependencies\OpenEXR\exrstdattr\main.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64

exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/exrstdattr.dir/main.cpp.i"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64\exrstdattr
	C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  > CMakeFiles\exrstdattr.dir\main.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\code\applications\mrViewer\dependencies\OpenEXR\exrstdattr\main.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64

exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/exrstdattr.dir/main.cpp.s"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64\exrstdattr
	C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\exrstdattr.dir\main.cpp.s /c D:\code\applications\mrViewer\dependencies\OpenEXR\exrstdattr\main.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64

exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj.requires:

.PHONY : exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj.requires

exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj.provides: exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj.requires
	$(MAKE) -f exrstdattr\CMakeFiles\exrstdattr.dir\build.make /nologo -$(MAKEFLAGS) exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj.provides.build
.PHONY : exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj.provides

exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj.provides.build: exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj


# Object files for target exrstdattr
exrstdattr_OBJECTS = \
"CMakeFiles\exrstdattr.dir\main.cpp.obj"

# External object files for target exrstdattr
exrstdattr_EXTERNAL_OBJECTS =

exrstdattr\exrstdattr.exe: exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj
exrstdattr\exrstdattr.exe: exrstdattr\CMakeFiles\exrstdattr.dir\build.make
exrstdattr\exrstdattr.exe: IlmImf\IlmImf-2_2.lib
exrstdattr\exrstdattr.exe: D:\code\applications\mrViewer\dependencies\zlib-1.2.8-win64\zlib.lib
exrstdattr\exrstdattr.exe: D:\code\applications\mrViewer\dependencies\zlib-1.2.8-win64\zlib.lib
exrstdattr\exrstdattr.exe: exrstdattr\CMakeFiles\exrstdattr.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable exrstdattr.exe"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64\exrstdattr
	"C:\Program Files\CMake\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\exrstdattr.dir --manifests  -- C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\link.exe /nologo @CMakeFiles\exrstdattr.dir\objects1.rsp @<<
 /out:exrstdattr.exe /implib:exrstdattr.lib /pdb:D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64\exrstdattr\exrstdattr.pdb /version:0.0   /machine:x64 /INCREMENTAL:NO /subsystem:console  -LIBPATH:d:\code\lib\Windows_64\lib  ..\IlmImf\IlmImf-2_2.lib IlmThread-2_2.lib Iex-2_2.lib Half.lib D:\code\applications\mrViewer\dependencies\zlib-1.2.8-win64\zlib.lib Imath-2_2.lib IlmThread-2_2.lib D:\code\applications\mrViewer\dependencies\zlib-1.2.8-win64\zlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64

# Rule to build all files generated by this target.
exrstdattr\CMakeFiles\exrstdattr.dir\build: exrstdattr\exrstdattr.exe

.PHONY : exrstdattr\CMakeFiles\exrstdattr.dir\build

exrstdattr\CMakeFiles\exrstdattr.dir\requires: exrstdattr\CMakeFiles\exrstdattr.dir\main.cpp.obj.requires

.PHONY : exrstdattr\CMakeFiles\exrstdattr.dir\requires

exrstdattr\CMakeFiles\exrstdattr.dir\clean:
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64\exrstdattr
	$(CMAKE_COMMAND) -P CMakeFiles\exrstdattr.dir\cmake_clean.cmake
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64
.PHONY : exrstdattr\CMakeFiles\exrstdattr.dir\clean

exrstdattr\CMakeFiles\exrstdattr.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" D:\code\applications\mrViewer\dependencies\OpenEXR D:\code\applications\mrViewer\dependencies\OpenEXR\exrstdattr D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64 D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64\exrstdattr D:\code\applications\mrViewer\dependencies\OpenEXR\build-win64\exrstdattr\CMakeFiles\exrstdattr.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : exrstdattr\CMakeFiles\exrstdattr.dir\depend

