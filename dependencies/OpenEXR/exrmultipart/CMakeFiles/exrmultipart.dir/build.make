# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canoncical targets will work.
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
CMAKE_COMMAND = "c:\Program Files (x86)\CMake 2.8\bin\cmake.exe"

# The command to remove a file.
RM = "c:\Program Files (x86)\CMake 2.8\bin\cmake.exe" -E remove -f

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = "c:\Program Files (x86)\CMake 2.8\bin\cmake-gui.exe"

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = F:\code\lib\openexr-git\OpenEXR

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = F:\code\lib\openexr-git\OpenEXR

# Include any dependencies generated for this target.
include exrmultipart\CMakeFiles\exrmultipart.dir\depend.make

# Include the progress variables for this target.
include exrmultipart\CMakeFiles\exrmultipart.dir\progress.make

# Include the compile flags for this target's objects.
include exrmultipart\CMakeFiles\exrmultipart.dir\flags.make

exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj: exrmultipart\CMakeFiles\exrmultipart.dir\flags.make
exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj: exrmultipart\exrmultipart.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report F:\code\lib\openexr-git\OpenEXR\CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object exrmultipart/CMakeFiles/exrmultipart.dir/exrmultipart.cpp.obj"
	cd F:\code\lib\openexr-git\OpenEXR\exrmultipart
	c:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  @<<
 /nologo $(CXX_FLAGS) $(CXX_DEFINES) /TP /FoCMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj /FdF:\code\lib\openexr-git\OpenEXR\exrmultipart\exrmultipart.pdb -c F:\code\lib\openexr-git\OpenEXR\exrmultipart\exrmultipart.cpp
<<
	cd F:\code\lib\openexr-git\OpenEXR

exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/exrmultipart.dir/exrmultipart.cpp.i"
	cd F:\code\lib\openexr-git\OpenEXR\exrmultipart
	c:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  > CMakeFiles\exrmultipart.dir\exrmultipart.cpp.i @<<
 /nologo $(CXX_FLAGS) $(CXX_DEFINES) /TP -E F:\code\lib\openexr-git\OpenEXR\exrmultipart\exrmultipart.cpp
<<
	cd F:\code\lib\openexr-git\OpenEXR

exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/exrmultipart.dir/exrmultipart.cpp.s"
	cd F:\code\lib\openexr-git\OpenEXR\exrmultipart
	c:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  @<<
 /nologo $(CXX_FLAGS) /TP /FAs /FoNUL /FaCMakeFiles\exrmultipart.dir\exrmultipart.cpp.s /c F:\code\lib\openexr-git\OpenEXR\exrmultipart\exrmultipart.cpp
<<
	cd F:\code\lib\openexr-git\OpenEXR

exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj.requires:
.PHONY : exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj.requires

exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj.provides: exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj.requires
	$(MAKE) -f exrmultipart\CMakeFiles\exrmultipart.dir\build.make /nologo -$(MAKEFLAGS) exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj.provides.build
.PHONY : exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj.provides

exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj.provides.build: exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj

# Object files for target exrmultipart
exrmultipart_OBJECTS = \
"CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj"

# External object files for target exrmultipart
exrmultipart_EXTERNAL_OBJECTS =

exrmultipart\exrmultipart.exe: exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj
exrmultipart\exrmultipart.exe: IlmImf\IlmImf-2_2.lib
exrmultipart\exrmultipart.exe: f:\code\lib\zlib-1.2.8-win64\zlib.lib
exrmultipart\exrmultipart.exe: f:\code\lib\zlib-1.2.8-win64\zlib.lib
exrmultipart\exrmultipart.exe: exrmultipart\CMakeFiles\exrmultipart.dir\build.make
exrmultipart\exrmultipart.exe: exrmultipart\CMakeFiles\exrmultipart.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable exrmultipart.exe"
	cd F:\code\lib\openexr-git\OpenEXR\exrmultipart
	"c:\Program Files (x86)\CMake 2.8\bin\cmake.exe" -E vs_link_exe c:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  /nologo @CMakeFiles\exrmultipart.dir\objects1.rsp @<<
  /DWIN32 /D_WINDOWS /W3 /Zm1000 /EHsc /GR /D_DEBUG /MDd /Zi /Ob0 /Od /RTC1 /Feexrmultipart.exe /FdF:\code\lib\openexr-git\OpenEXR\exrmultipart\exrmultipart.pdb -link /implib:exrmultipart.lib /version:0.0   /STACK:10000000 /machine:x64  /debug /INCREMENTAL /subsystem:console  -LIBPATH:f:\code\lib\Windows_64\lib ..\IlmImf\IlmImf-2_2.lib IlmThread-2_2.lib Iex-2_2.lib Half.lib f:\code\lib\zlib-1.2.8-win64\zlib.lib Imath-2_2.lib IlmThread-2_2.lib f:\code\lib\zlib-1.2.8-win64\zlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib  
<<
	cd F:\code\lib\openexr-git\OpenEXR

# Rule to build all files generated by this target.
exrmultipart\CMakeFiles\exrmultipart.dir\build: exrmultipart\exrmultipart.exe
.PHONY : exrmultipart\CMakeFiles\exrmultipart.dir\build

exrmultipart\CMakeFiles\exrmultipart.dir\requires: exrmultipart\CMakeFiles\exrmultipart.dir\exrmultipart.cpp.obj.requires
.PHONY : exrmultipart\CMakeFiles\exrmultipart.dir\requires

exrmultipart\CMakeFiles\exrmultipart.dir\clean:
	cd F:\code\lib\openexr-git\OpenEXR\exrmultipart
	$(CMAKE_COMMAND) -P CMakeFiles\exrmultipart.dir\cmake_clean.cmake
	cd F:\code\lib\openexr-git\OpenEXR
.PHONY : exrmultipart\CMakeFiles\exrmultipart.dir\clean

exrmultipart\CMakeFiles\exrmultipart.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" F:\code\lib\openexr-git\OpenEXR F:\code\lib\openexr-git\OpenEXR\exrmultipart F:\code\lib\openexr-git\OpenEXR F:\code\lib\openexr-git\OpenEXR\exrmultipart F:\code\lib\openexr-git\OpenEXR\exrmultipart\CMakeFiles\exrmultipart.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : exrmultipart\CMakeFiles\exrmultipart.dir\depend

