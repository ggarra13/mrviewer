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
include CMakeFiles\One.dir\depend.make

# Include the progress variables for this target.
include CMakeFiles\One.dir\progress.make

# Include the compile flags for this target's objects.
include CMakeFiles\One.dir\flags.make

CMakeFiles\One.dir\progs\demos\One\one.c.obj: CMakeFiles\One.dir\flags.make
CMakeFiles\One.dir\progs\demos\One\one.c.obj: ..\progs\demos\One\one.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/One.dir/progs/demos/One/one.c.obj"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) /FoCMakeFiles\One.dir\progs\demos\One\one.c.obj /FdCMakeFiles\One.dir\ /FS -c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\One\one.c
<<

CMakeFiles\One.dir\progs\demos\One\one.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/One.dir/progs/demos/One/one.c.i"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  > CMakeFiles\One.dir\progs\demos\One\one.c.i @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\One\one.c
<<

CMakeFiles\One.dir\progs\demos\One\one.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/One.dir/progs/demos/One/one.c.s"
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) /FoNUL /FAs /FaCMakeFiles\One.dir\progs\demos\One\one.c.s /c D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\One\one.c
<<

CMakeFiles\One.dir\progs\demos\One\one.c.obj.requires:

.PHONY : CMakeFiles\One.dir\progs\demos\One\one.c.obj.requires

CMakeFiles\One.dir\progs\demos\One\one.c.obj.provides: CMakeFiles\One.dir\progs\demos\One\one.c.obj.requires
	$(MAKE) -f CMakeFiles\One.dir\build.make /nologo -$(MAKEFLAGS) CMakeFiles\One.dir\progs\demos\One\one.c.obj.provides.build
.PHONY : CMakeFiles\One.dir\progs\demos\One\one.c.obj.provides

CMakeFiles\One.dir\progs\demos\One\one.c.obj.provides.build: CMakeFiles\One.dir\progs\demos\One\one.c.obj


CMakeFiles\One.dir\progs\demos\One\one.rc.res: CMakeFiles\One.dir\flags.make
CMakeFiles\One.dir\progs\demos\One\one.rc.res: ..\progs\demos\One\one.rc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building RC object CMakeFiles/One.dir/progs/demos/One/one.rc.res"
	C:\PROGRA~2\WI3CF2~1\8.1\bin\x86\rc.exe  $(RC_DEFINES) $(RC_INCLUDES) $(RC_FLAGS) /foCMakeFiles\One.dir\progs\demos\One\one.rc.res D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\progs\demos\One\one.rc

CMakeFiles\One.dir\progs\demos\One\one.rc.res.requires:

.PHONY : CMakeFiles\One.dir\progs\demos\One\one.rc.res.requires

CMakeFiles\One.dir\progs\demos\One\one.rc.res.provides: CMakeFiles\One.dir\progs\demos\One\one.rc.res.requires
	$(MAKE) -f CMakeFiles\One.dir\build.make /nologo -$(MAKEFLAGS) CMakeFiles\One.dir\progs\demos\One\one.rc.res.provides.build
.PHONY : CMakeFiles\One.dir\progs\demos\One\one.rc.res.provides

CMakeFiles\One.dir\progs\demos\One\one.rc.res.provides.build: CMakeFiles\One.dir\progs\demos\One\one.rc.res


# Object files for target One
One_OBJECTS = \
"CMakeFiles\One.dir\progs\demos\One\one.c.obj" \
"CMakeFiles\One.dir\progs\demos\One\one.rc.res"

# External object files for target One
One_EXTERNAL_OBJECTS =

bin\One.exe: CMakeFiles\One.dir\progs\demos\One\one.c.obj
bin\One.exe: CMakeFiles\One.dir\progs\demos\One\one.rc.res
bin\One.exe: CMakeFiles\One.dir\build.make
bin\One.exe: lib\freeglut.lib
bin\One.exe: CMakeFiles\One.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable bin\One.exe"
	"C:\Program Files\CMake\bin\cmake.exe" -E vs_link_exe --intdir=CMakeFiles\One.dir --manifests  -- C:\PROGRA~2\MICROS~2.0\VC\bin\link.exe /nologo @CMakeFiles\One.dir\objects1.rsp @<<
 /out:bin\One.exe /implib:lib\One.lib /pdb:D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\bin\One.pdb /version:0.0   /machine:X86 /INCREMENTAL:NO /subsystem:console  glu32.lib opengl32.lib winmm.lib lib\freeglut.lib opengl32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
<<

# Rule to build all files generated by this target.
CMakeFiles\One.dir\build: bin\One.exe

.PHONY : CMakeFiles\One.dir\build

CMakeFiles\One.dir\requires: CMakeFiles\One.dir\progs\demos\One\one.c.obj.requires
CMakeFiles\One.dir\requires: CMakeFiles\One.dir\progs\demos\One\one.rc.res.requires

.PHONY : CMakeFiles\One.dir\requires

CMakeFiles\One.dir\clean:
	$(CMAKE_COMMAND) -P CMakeFiles\One.dir\cmake_clean.cmake
.PHONY : CMakeFiles\One.dir\clean

CMakeFiles\One.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" D:\code\applications\mrViewer\dependencies\freeglut-3.0.0 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32 D:\code\applications\mrViewer\dependencies\freeglut-3.0.0\build-win32\CMakeFiles\One.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles\One.dir\depend

