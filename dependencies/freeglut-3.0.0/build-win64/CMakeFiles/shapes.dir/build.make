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
CMAKE_BINARY_DIR = D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win64

# Include any dependencies generated for this target.
include CMakeFiles\shapes.dir\depend.make

# Include the progress variables for this target.
include CMakeFiles\shapes.dir\progress.make

# Include the compile flags for this target's objects.
include CMakeFiles\shapes.dir\flags.make

CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj: CMakeFiles\shapes.dir\flags.make
CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj: ..\progs\demos\shapes\shapes.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win64\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/shapes.dir/progs/demos/shapes/shapes.c.obj"
	C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) /FoCMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj /FdCMakeFiles\shapes.dir\ /FS -c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\shapes\shapes.c
<<

CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/shapes.dir/progs/demos/shapes/shapes.c.i"
	C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  > CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.i @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\shapes\shapes.c
<<

CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/shapes.dir/progs/demos/shapes/shapes.c.s"
	C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) /FoNUL /FAs /FaCMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.s /c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\shapes\shapes.c
<<

CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj.requires:

.PHONY : CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj.requires

CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj.provides: CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj.requires
	$(MAKE) -f CMakeFiles\shapes.dir\build.make /nologo -$(MAKEFLAGS) CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj.provides.build
.PHONY : CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj.provides

CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj.provides.build: CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj


CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj: CMakeFiles\shapes.dir\flags.make
CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj: ..\progs\demos\shapes\glmatrix.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win64\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/shapes.dir/progs/demos/shapes/glmatrix.c.obj"
	C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) /FoCMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj /FdCMakeFiles\shapes.dir\ /FS -c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\shapes\glmatrix.c
<<

CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/shapes.dir/progs/demos/shapes/glmatrix.c.i"
	C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  > CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.i @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\shapes\glmatrix.c
<<

CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/shapes.dir/progs/demos/shapes/glmatrix.c.s"
	C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\cl.exe  @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) /FoNUL /FAs /FaCMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.s /c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\shapes\glmatrix.c
<<

CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj.requires:

.PHONY : CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj.requires

CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj.provides: CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj.requires
	$(MAKE) -f CMakeFiles\shapes.dir\build.make /nologo -$(MAKEFLAGS) CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj.provides.build
.PHONY : CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj.provides

CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj.provides.build: CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj


# Object files for target shapes
shapes_OBJECTS = \
"CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj" \
"CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj"

# External object files for target shapes
shapes_EXTERNAL_OBJECTS =

bin\shapes.exe: CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj
bin\shapes.exe: CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj
bin\shapes.exe: CMakeFiles\shapes.dir\build.make
bin\shapes.exe: lib\freeglut.lib
bin\shapes.exe: CMakeFiles\shapes.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win64\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable bin\shapes.exe"
	"C:\Program Files\CMake\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\shapes.dir --manifests  -- C:\PROGRA~2\MICROS~1.0\VC\bin\amd64\link.exe /nologo @CMakeFiles\shapes.dir\objects1.rsp @<<
 /out:bin\shapes.exe /implib:lib\shapes.lib /pdb:D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win64\bin\shapes.pdb /version:0.0   /machine:x64 /INCREMENTAL:NO /subsystem:console  glu32.lib opengl32.lib winmm.lib lib\freeglut.lib opengl32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<

# Rule to build all files generated by this target.
CMakeFiles\shapes.dir\build: bin\shapes.exe

.PHONY : CMakeFiles\shapes.dir\build

CMakeFiles\shapes.dir\requires: CMakeFiles\shapes.dir\progs\demos\shapes\shapes.c.obj.requires
CMakeFiles\shapes.dir\requires: CMakeFiles\shapes.dir\progs\demos\shapes\glmatrix.c.obj.requires

.PHONY : CMakeFiles\shapes.dir\requires

CMakeFiles\shapes.dir\clean:
	$(CMAKE_COMMAND) -P CMakeFiles\shapes.dir\cmake_clean.cmake
.PHONY : CMakeFiles\shapes.dir\clean

CMakeFiles\shapes.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" D:\code\applications\mrViewer\dependencies\freeglut-3.0.0 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win64 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win64 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win64\CMakeFiles\shapes.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles\shapes.dir\depend

