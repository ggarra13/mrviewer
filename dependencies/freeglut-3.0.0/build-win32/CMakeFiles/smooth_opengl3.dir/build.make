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
include CMakeFiles\smooth_opengl3.dir\depend.make

# Include the progress variables for this target.
include CMakeFiles\smooth_opengl3.dir\progress.make

# Include the compile flags for this target's objects.
include CMakeFiles\smooth_opengl3.dir\flags.make

CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj: CMakeFiles\smooth_opengl3.dir\flags.make
CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj: ..\progs\demos\smooth_opengl3\smooth_opengl3.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/smooth_opengl3.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.obj"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) /FoCMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj /FdCMakeFiles\smooth_opengl3.dir\ /FS -c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\smooth_opengl3\smooth_opengl3.c
<<

CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/smooth_opengl3.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.i"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  > CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.i @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\smooth_opengl3\smooth_opengl3.c
<<

CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/smooth_opengl3.dir/progs/demos/smooth_opengl3/smooth_opengl3.c.s"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) /FoNUL /FAs /FaCMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.s /c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\smooth_opengl3\smooth_opengl3.c
<<

CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj.requires:

.PHONY : CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj.requires

CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj.provides: CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj.requires
	$(MAKE) -f CMakeFiles\smooth_opengl3.dir\build.make /nologo -$(MAKEFLAGS) CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj.provides.build
.PHONY : CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj.provides

CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj.provides.build: CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj


# Object files for target smooth_opengl3
smooth_opengl3_OBJECTS = \
"CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj"

# External object files for target smooth_opengl3
smooth_opengl3_EXTERNAL_OBJECTS =

bin\smooth_opengl3.exe: CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj
bin\smooth_opengl3.exe: CMakeFiles\smooth_opengl3.dir\build.make
bin\smooth_opengl3.exe: lib\freeglut.lib
bin\smooth_opengl3.exe: CMakeFiles\smooth_opengl3.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable bin\smooth_opengl3.exe"
	"C:\Program Files\CMake\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\smooth_opengl3.dir --manifests  -- C:\PROGRA~2\MICROS~2.0\VC\bin\link.exe /nologo @CMakeFiles\smooth_opengl3.dir\objects1.rsp @<<
 /out:bin\smooth_opengl3.exe /implib:lib\smooth_opengl3.lib /pdb:D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\bin\smooth_opengl3.pdb /version:0.0   /machine:X86 /INCREMENTAL:NO /subsystem:console  glu32.lib opengl32.lib winmm.lib lib\freeglut.lib opengl32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<

# Rule to build all files generated by this target.
CMakeFiles\smooth_opengl3.dir\build: bin\smooth_opengl3.exe

.PHONY : CMakeFiles\smooth_opengl3.dir\build

CMakeFiles\smooth_opengl3.dir\requires: CMakeFiles\smooth_opengl3.dir\progs\demos\smooth_opengl3\smooth_opengl3.c.obj.requires

.PHONY : CMakeFiles\smooth_opengl3.dir\requires

CMakeFiles\smooth_opengl3.dir\clean:
	$(CMAKE_COMMAND) -P CMakeFiles\smooth_opengl3.dir\cmake_clean.cmake
.PHONY : CMakeFiles\smooth_opengl3.dir\clean

CMakeFiles\smooth_opengl3.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" D:\code\applications\mrViewer\dependencies\freeglut-3.0.0 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles\smooth_opengl3.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles\smooth_opengl3.dir\depend

