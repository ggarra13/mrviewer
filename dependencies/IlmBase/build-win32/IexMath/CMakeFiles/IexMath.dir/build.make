# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.1

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
CMAKE_COMMAND = "c:\Program Files (x86)\cmake-3.1.2-win32-x86\bin\cmake.exe"

# The command to remove a file.
RM = "c:\Program Files (x86)\cmake-3.1.2-win32-x86\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = F:\code\lib\openexr-git\IlmBase

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = F:\code\lib\openexr-git\IlmBase\build-win32

# Include any dependencies generated for this target.
include IexMath\CMakeFiles\IexMath.dir\depend.make

# Include the progress variables for this target.
include IexMath\CMakeFiles\IexMath.dir\progress.make

# Include the compile flags for this target's objects.
include IexMath\CMakeFiles\IexMath.dir\flags.make

IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj: IexMath\CMakeFiles\IexMath.dir\flags.make
IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj: ..\IexMath\IexMathFloatExc.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report F:\code\lib\openexr-git\IlmBase\build-win32\CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object IexMath/CMakeFiles/IexMath.dir/IexMathFloatExc.cpp.obj"
	cd F:\code\lib\openexr-git\IlmBase\build-win32\IexMath
	c:\PROGRA~2\MICROS~4.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj /FdCMakeFiles\IexMath.dir\ /FS -c F:\code\lib\openexr-git\IlmBase\IexMath\IexMathFloatExc.cpp
<<
	cd F:\code\lib\openexr-git\IlmBase\build-win32

IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/IexMath.dir/IexMathFloatExc.cpp.i"
	cd F:\code\lib\openexr-git\IlmBase\build-win32\IexMath
	c:\PROGRA~2\MICROS~4.0\VC\bin\cl.exe  > CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E F:\code\lib\openexr-git\IlmBase\IexMath\IexMathFloatExc.cpp
<<
	cd F:\code\lib\openexr-git\IlmBase\build-win32

IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/IexMath.dir/IexMathFloatExc.cpp.s"
	cd F:\code\lib\openexr-git\IlmBase\build-win32\IexMath
	c:\PROGRA~2\MICROS~4.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\IexMath.dir\IexMathFloatExc.cpp.s /c F:\code\lib\openexr-git\IlmBase\IexMath\IexMathFloatExc.cpp
<<
	cd F:\code\lib\openexr-git\IlmBase\build-win32

IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj.requires:
.PHONY : IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj.requires

IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj.provides: IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj.requires
	$(MAKE) -f IexMath\CMakeFiles\IexMath.dir\build.make /nologo -$(MAKEFLAGS) IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj.provides.build
.PHONY : IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj.provides

IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj.provides.build: IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj

IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj: IexMath\CMakeFiles\IexMath.dir\flags.make
IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj: ..\IexMath\IexMathFpu.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report F:\code\lib\openexr-git\IlmBase\build-win32\CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object IexMath/CMakeFiles/IexMath.dir/IexMathFpu.cpp.obj"
	cd F:\code\lib\openexr-git\IlmBase\build-win32\IexMath
	c:\PROGRA~2\MICROS~4.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\IexMath.dir\IexMathFpu.cpp.obj /FdCMakeFiles\IexMath.dir\ /FS -c F:\code\lib\openexr-git\IlmBase\IexMath\IexMathFpu.cpp
<<
	cd F:\code\lib\openexr-git\IlmBase\build-win32

IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/IexMath.dir/IexMathFpu.cpp.i"
	cd F:\code\lib\openexr-git\IlmBase\build-win32\IexMath
	c:\PROGRA~2\MICROS~4.0\VC\bin\cl.exe  > CMakeFiles\IexMath.dir\IexMathFpu.cpp.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E F:\code\lib\openexr-git\IlmBase\IexMath\IexMathFpu.cpp
<<
	cd F:\code\lib\openexr-git\IlmBase\build-win32

IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/IexMath.dir/IexMathFpu.cpp.s"
	cd F:\code\lib\openexr-git\IlmBase\build-win32\IexMath
	c:\PROGRA~2\MICROS~4.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\IexMath.dir\IexMathFpu.cpp.s /c F:\code\lib\openexr-git\IlmBase\IexMath\IexMathFpu.cpp
<<
	cd F:\code\lib\openexr-git\IlmBase\build-win32

IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj.requires:
.PHONY : IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj.requires

IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj.provides: IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj.requires
	$(MAKE) -f IexMath\CMakeFiles\IexMath.dir\build.make /nologo -$(MAKEFLAGS) IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj.provides.build
.PHONY : IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj.provides

IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj.provides.build: IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj

# Object files for target IexMath
IexMath_OBJECTS = \
"CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj" \
"CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj"

# External object files for target IexMath
IexMath_EXTERNAL_OBJECTS =

IexMath\IexMath-2_2.dll: IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj
IexMath\IexMath-2_2.dll: IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj
IexMath\IexMath-2_2.dll: IexMath\CMakeFiles\IexMath.dir\build.make
IexMath\IexMath-2_2.dll: Iex\Iex-2_2.lib
IexMath\IexMath-2_2.dll: IexMath\CMakeFiles\IexMath.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX shared library IexMath-2_2.dll"
	cd F:\code\lib\openexr-git\IlmBase\build-win32\IexMath
	"c:\Program Files (x86)\cmake-3.1.2-win32-x86\bin\cmake.exe" -E vs_link_dll c:\PROGRA~2\MICROS~4.0\VC\bin\link.exe /nologo @CMakeFiles\IexMath.dir\objects1.rsp @<<
 /out:IexMath-2_2.dll /implib:IexMath-2_2.lib /pdb:F:\code\lib\openexr-git\IlmBase\build-win32\IexMath\IexMath-2_2.pdb /dll /version:12.0  /machine:X86 /INCREMENTAL:NO ..\Iex\Iex-2_2.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib  
<<
	cd F:\code\lib\openexr-git\IlmBase\build-win32

# Rule to build all files generated by this target.
IexMath\CMakeFiles\IexMath.dir\build: IexMath\IexMath-2_2.dll
.PHONY : IexMath\CMakeFiles\IexMath.dir\build

IexMath\CMakeFiles\IexMath.dir\requires: IexMath\CMakeFiles\IexMath.dir\IexMathFloatExc.cpp.obj.requires
IexMath\CMakeFiles\IexMath.dir\requires: IexMath\CMakeFiles\IexMath.dir\IexMathFpu.cpp.obj.requires
.PHONY : IexMath\CMakeFiles\IexMath.dir\requires

IexMath\CMakeFiles\IexMath.dir\clean:
	cd F:\code\lib\openexr-git\IlmBase\build-win32\IexMath
	$(CMAKE_COMMAND) -P CMakeFiles\IexMath.dir\cmake_clean.cmake
	cd F:\code\lib\openexr-git\IlmBase\build-win32
.PHONY : IexMath\CMakeFiles\IexMath.dir\clean

IexMath\CMakeFiles\IexMath.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" F:\code\lib\openexr-git\IlmBase F:\code\lib\openexr-git\IlmBase\IexMath F:\code\lib\openexr-git\IlmBase\build-win32 F:\code\lib\openexr-git\IlmBase\build-win32\IexMath F:\code\lib\openexr-git\IlmBase\build-win32\IexMath\CMakeFiles\IexMath.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : IexMath\CMakeFiles\IexMath.dir\depend

