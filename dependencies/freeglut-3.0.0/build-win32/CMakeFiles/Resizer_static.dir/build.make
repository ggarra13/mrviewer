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
CMAKE_SOURCE_DIR = D:\code\applications\mrViewer\dependencies\freeglut-3.0.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32

# Include any dependencies generated for this target.
include CMakeFiles\Resizer_static.dir\depend.make

# Include the progress variables for this target.
include CMakeFiles\Resizer_static.dir\progress.make

# Include the compile flags for this target's objects.
include CMakeFiles\Resizer_static.dir\flags.make

CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj: CMakeFiles\Resizer_static.dir\flags.make
CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj: ..\progs\demos\Resizer\Resizer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Resizer_static.dir/progs/demos/Resizer/Resizer.cpp.obj"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoCMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj /FdCMakeFiles\Resizer_static.dir\ /FS -c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\Resizer\Resizer.cpp
<<

CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Resizer_static.dir/progs/demos/Resizer/Resizer.cpp.i"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  > CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.i @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\Resizer\Resizer.cpp
<<

CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Resizer_static.dir/progs/demos/Resizer/Resizer.cpp.s"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) /FoNUL /FAs /FaCMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.s /c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\Resizer\Resizer.cpp
<<

CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj.requires:

.PHONY : CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj.requires

CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj.provides: CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj.requires
	$(MAKE) -f CMakeFiles\Resizer_static.dir\build.make /nologo -$(MAKEFLAGS) CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj.provides.build
.PHONY : CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj.provides

CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj.provides.build: CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj


# Object files for target Resizer_static
Resizer_static_OBJECTS = \
"CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj"

# External object files for target Resizer_static
Resizer_static_EXTERNAL_OBJECTS =

bin\Resizer_static.exe: CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj
bin\Resizer_static.exe: CMakeFiles\Resizer_static.dir\build.make
bin\Resizer_static.exe: lib\freeglut_static.lib
bin\Resizer_static.exe: CMakeFiles\Resizer_static.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable bin\Resizer_static.exe"
	"C:\Program Files\CMake\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\Resizer_static.dir --manifests  -- C:\PROGRA~2\MICROS~2.0\VC\bin\link.exe /nologo @CMakeFiles\Resizer_static.dir\objects1.rsp @<<
 /out:bin\Resizer_static.exe /implib:lib\Resizer_static.lib /pdb:D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\bin\Resizer_static.pdb /version:0.0   /machine:X86 /INCREMENTAL:NO /subsystem:console  glu32.lib opengl32.lib winmm.lib lib\freeglut_static.lib opengl32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<

# Rule to build all files generated by this target.
CMakeFiles\Resizer_static.dir\build: bin\Resizer_static.exe

.PHONY : CMakeFiles\Resizer_static.dir\build

CMakeFiles\Resizer_static.dir\requires: CMakeFiles\Resizer_static.dir\progs\demos\Resizer\Resizer.cpp.obj.requires

.PHONY : CMakeFiles\Resizer_static.dir\requires

CMakeFiles\Resizer_static.dir\clean:
	$(CMAKE_COMMAND) -P CMakeFiles\Resizer_static.dir\cmake_clean.cmake
.PHONY : CMakeFiles\Resizer_static.dir\clean

CMakeFiles\Resizer_static.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" D:\code\applications\mrViewer\dependencies\freeglut-3.0.0 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles\Resizer_static.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles\Resizer_static.dir\depend

