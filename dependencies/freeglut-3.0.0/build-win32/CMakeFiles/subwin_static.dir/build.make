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
include CMakeFiles\subwin_static.dir\depend.make

# Include the progress variables for this target.
include CMakeFiles\subwin_static.dir\progress.make

# Include the compile flags for this target's objects.
include CMakeFiles\subwin_static.dir\flags.make

CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj: CMakeFiles\subwin_static.dir\flags.make
CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj: ..\progs\demos\subwin\subwin.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/subwin_static.dir/progs/demos/subwin/subwin.c.obj"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) /FoCMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj /FdCMakeFiles\subwin_static.dir\ /FS -c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\subwin\subwin.c
<<

CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/subwin_static.dir/progs/demos/subwin/subwin.c.i"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  > CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.i @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\subwin\subwin.c
<<

CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/subwin_static.dir/progs/demos/subwin/subwin.c.s"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) /FoNUL /FAs /FaCMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.s /c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\subwin\subwin.c
<<

CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj.requires:

.PHONY : CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj.requires

CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj.provides: CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj.requires
	$(MAKE) -f CMakeFiles\subwin_static.dir\build.make /nologo -$(MAKEFLAGS) CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj.provides.build
.PHONY : CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj.provides

CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj.provides.build: CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj


# Object files for target subwin_static
subwin_static_OBJECTS = \
"CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj"

# External object files for target subwin_static
subwin_static_EXTERNAL_OBJECTS =

bin\subwin_static.exe: CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj
bin\subwin_static.exe: CMakeFiles\subwin_static.dir\build.make
bin\subwin_static.exe: lib\freeglut_static.lib
bin\subwin_static.exe: CMakeFiles\subwin_static.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable bin\subwin_static.exe"
	"C:\Program Files\CMake\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\subwin_static.dir --manifests  -- C:\PROGRA~2\MICROS~2.0\VC\bin\link.exe /nologo @CMakeFiles\subwin_static.dir\objects1.rsp @<<
 /out:bin\subwin_static.exe /implib:lib\subwin_static.lib /pdb:D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\bin\subwin_static.pdb /version:0.0   /machine:X86 /INCREMENTAL:NO /subsystem:console  glu32.lib opengl32.lib winmm.lib lib\freeglut_static.lib opengl32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<

# Rule to build all files generated by this target.
CMakeFiles\subwin_static.dir\build: bin\subwin_static.exe

.PHONY : CMakeFiles\subwin_static.dir\build

CMakeFiles\subwin_static.dir\requires: CMakeFiles\subwin_static.dir\progs\demos\subwin\subwin.c.obj.requires

.PHONY : CMakeFiles\subwin_static.dir\requires

CMakeFiles\subwin_static.dir\clean:
	$(CMAKE_COMMAND) -P CMakeFiles\subwin_static.dir\cmake_clean.cmake
.PHONY : CMakeFiles\subwin_static.dir\clean

CMakeFiles\subwin_static.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" D:\code\applications\mrViewer\dependencies\freeglut-3.0.0 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles\subwin_static.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles\subwin_static.dir\depend

