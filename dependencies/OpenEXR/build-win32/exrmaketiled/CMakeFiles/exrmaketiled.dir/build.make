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
CMAKE_BINARY_DIR = D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

# Include any dependencies generated for this target.
include exrmaketiled\CMakeFiles\exrmaketiled.dir\depend.make

# Include the progress variables for this target.
include exrmaketiled\CMakeFiles\exrmaketiled.dir\progress.make

# Include the compile flags for this target's objects.
include exrmaketiled\CMakeFiles\exrmaketiled.dir\flags.make

exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj: exrmaketiled\CMakeFiles\exrmaketiled.dir\flags.make
exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj: ..\exrmaketiled\makeTiled.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object exrmaketiled/CMakeFiles/exrmaketiled.dir/makeTiled.cpp.obj"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoCMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj /FdCMakeFiles\exrmaketiled.dir\ /FS -c D:\code\applications\mrViewer\dependencies\OpenEXR\exrmaketiled\makeTiled.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/exrmaketiled.dir/makeTiled.cpp.i"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  > CMakeFiles\exrmaketiled.dir\makeTiled.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\code\applications\mrViewer\dependencies\OpenEXR\exrmaketiled\makeTiled.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/exrmaketiled.dir/makeTiled.cpp.s"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\exrmaketiled.dir\makeTiled.cpp.s /c D:\code\applications\mrViewer\dependencies\OpenEXR\exrmaketiled\makeTiled.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj.requires:

.PHONY : exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj.requires

exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj.provides: exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj.requires
	$(MAKE) -f exrmaketiled\CMakeFiles\exrmaketiled.dir\build.make /nologo -$(MAKEFLAGS) exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj.provides.build
.PHONY : exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj.provides

exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj.provides.build: exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj


exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj: exrmaketiled\CMakeFiles\exrmaketiled.dir\flags.make
exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj: ..\exrmaketiled\main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object exrmaketiled/CMakeFiles/exrmaketiled.dir/main.cpp.obj"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoCMakeFiles\exrmaketiled.dir\main.cpp.obj /FdCMakeFiles\exrmaketiled.dir\ /FS -c D:\code\applications\mrViewer\dependencies\OpenEXR\exrmaketiled\main.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/exrmaketiled.dir/main.cpp.i"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  > CMakeFiles\exrmaketiled.dir\main.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\code\applications\mrViewer\dependencies\OpenEXR\exrmaketiled\main.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/exrmaketiled.dir/main.cpp.s"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\exrmaketiled.dir\main.cpp.s /c D:\code\applications\mrViewer\dependencies\OpenEXR\exrmaketiled\main.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj.requires:

.PHONY : exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj.requires

exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj.provides: exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj.requires
	$(MAKE) -f exrmaketiled\CMakeFiles\exrmaketiled.dir\build.make /nologo -$(MAKEFLAGS) exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj.provides.build
.PHONY : exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj.provides

exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj.provides.build: exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj


exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj: exrmaketiled\CMakeFiles\exrmaketiled.dir\flags.make
exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj: ..\exrmaketiled\Image.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object exrmaketiled/CMakeFiles/exrmaketiled.dir/Image.cpp.obj"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoCMakeFiles\exrmaketiled.dir\Image.cpp.obj /FdCMakeFiles\exrmaketiled.dir\ /FS -c D:\code\applications\mrViewer\dependencies\OpenEXR\exrmaketiled\Image.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/exrmaketiled.dir/Image.cpp.i"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  > CMakeFiles\exrmaketiled.dir\Image.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\code\applications\mrViewer\dependencies\OpenEXR\exrmaketiled\Image.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/exrmaketiled.dir/Image.cpp.s"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\exrmaketiled.dir\Image.cpp.s /c D:\code\applications\mrViewer\dependencies\OpenEXR\exrmaketiled\Image.cpp
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj.requires:

.PHONY : exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj.requires

exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj.provides: exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj.requires
	$(MAKE) -f exrmaketiled\CMakeFiles\exrmaketiled.dir\build.make /nologo -$(MAKEFLAGS) exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj.provides.build
.PHONY : exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj.provides

exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj.provides.build: exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj


# Object files for target exrmaketiled
exrmaketiled_OBJECTS = \
"CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj" \
"CMakeFiles\exrmaketiled.dir\main.cpp.obj" \
"CMakeFiles\exrmaketiled.dir\Image.cpp.obj"

# External object files for target exrmaketiled
exrmaketiled_EXTERNAL_OBJECTS =

exrmaketiled\exrmaketiled.exe: exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj
exrmaketiled\exrmaketiled.exe: exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj
exrmaketiled\exrmaketiled.exe: exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj
exrmaketiled\exrmaketiled.exe: exrmaketiled\CMakeFiles\exrmaketiled.dir\build.make
exrmaketiled\exrmaketiled.exe: IlmImf\IlmImf-2_2.lib
exrmaketiled\exrmaketiled.exe: d:\code\lib\Windows_32\lib\zdll.lib
exrmaketiled\exrmaketiled.exe: d:\code\lib\Windows_32\lib\zdll.lib
exrmaketiled\exrmaketiled.exe: exrmaketiled\CMakeFiles\exrmaketiled.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable exrmaketiled.exe"
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	"C:\Program Files\CMake\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\exrmaketiled.dir --manifests  -- C:\PROGRA~2\MICROS~2.0\VC\bin\link.exe /nologo @CMakeFiles\exrmaketiled.dir\objects1.rsp @<<
 /out:exrmaketiled.exe /implib:exrmaketiled.lib /pdb:D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled\exrmaketiled.pdb /version:0.0   /machine:X86 /INCREMENTAL:NO /subsystem:console  -LIBPATH:d:\code\lib\Windows_32\lib  ..\IlmImf\IlmImf-2_2.lib IlmThread-2_2.lib Iex-2_2.lib Half.lib d:\code\lib\Windows_32\lib\zdll.lib Imath-2_2.lib IlmThread-2_2.lib d:\code\lib\Windows_32\lib\zdll.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32

# Rule to build all files generated by this target.
exrmaketiled\CMakeFiles\exrmaketiled.dir\build: exrmaketiled\exrmaketiled.exe

.PHONY : exrmaketiled\CMakeFiles\exrmaketiled.dir\build

exrmaketiled\CMakeFiles\exrmaketiled.dir\requires: exrmaketiled\CMakeFiles\exrmaketiled.dir\makeTiled.cpp.obj.requires
exrmaketiled\CMakeFiles\exrmaketiled.dir\requires: exrmaketiled\CMakeFiles\exrmaketiled.dir\main.cpp.obj.requires
exrmaketiled\CMakeFiles\exrmaketiled.dir\requires: exrmaketiled\CMakeFiles\exrmaketiled.dir\Image.cpp.obj.requires

.PHONY : exrmaketiled\CMakeFiles\exrmaketiled.dir\requires

exrmaketiled\CMakeFiles\exrmaketiled.dir\clean:
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled
	$(CMAKE_COMMAND) -P CMakeFiles\exrmaketiled.dir\cmake_clean.cmake
	cd D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32
.PHONY : exrmaketiled\CMakeFiles\exrmaketiled.dir\clean

exrmaketiled\CMakeFiles\exrmaketiled.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" D:\code\applications\mrViewer\dependencies\OpenEXR D:\code\applications\mrViewer\dependencies\OpenEXR\exrmaketiled D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32 D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled D:\code\applications\mrViewer\dependencies\OpenEXR\build-win32\exrmaketiled\CMakeFiles\exrmaketiled.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : exrmaketiled\CMakeFiles\exrmaketiled.dir\depend

