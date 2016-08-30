[ The files in this directory are to support the ImageMagick
  setup.exe-style installer package. The following are the instructions
  foro how to build a Q:8 DLL-based distribution installer package
  using Visual C++
  7.0. ]

	   Steps for building VisualMagick Distribution Package

0) Install prerequisite software

  a) Download and install the Inno Setup Compiler from
      "http://www.jrsoftware.org/isinfo.php".

  b) Download and install Strawberry Perl from http://www.strawberryperl.com.

1) Open workspace VisualMagick\configure\configure.sln by double-clicking
   from Windows Explorer.

   a) Select "Rebuild All"
   b) Click on '!' icon to run configure program
   c) Select DLL build
   d) Uncheck "Use X11 Stubs" and check "Build demo and test programs"
   e) Click on Edit "magick-config.h" and ensure that
      UseInstalledMagick and ProvideDllMain are defined.
   f) Finish remaining configure wizard screens to complete.
   g) File -> "Close Workspace"

2) Open workspace VisualMagick\VisualDynamicMT.sln by double-clicking from
   Windows Explorer or opening workpace via Visual C++ dialog.

   a) Build -> "Set Active Configuration" -> "All - Win32 Release" -> OK
   b) Build -> "Rebuild All"

3) Build ImageMagickObject

   a) cd contrib\win32\ATL7\ImageMagickObject
   b) BuildImageMagickObject release
   c) cd ..\..\..\..

4) Open Windows Command Shell Window

   a) cd PerlMagick
   b) nmake clean (only if this is a rebuild)
   c) perl Makefile.nt
   d) nmake release

NOTE: access to nmake requires that there be a path to it. Depending n
how the install of Visual Studio was done, this may not be the case.
Visual Studio provides a batch script in VC98\Bin called VCVARS32.BAT
that can be used to do this manually after you open up a command
prompt.

5) Open VisualMagick\installer\im-dll-8.iss by double-clicking from
   Windows Explorer.

   a) File -> Compile
   b) Test install by clicking on green triangle

6)
   a) cd PerlMagick
   b) nmake test
      All tests must pass!

7) 
   a) cd VisualMagick\tests
   b) run_rwfile.bat
      All tests must pass!
   c) run_rwblob.bat
      All tests must pass!

8)
   a) cd Magick++/tests
   b) run_tests.bat
      All tests must pass!

9)
   a) cd Magick++/demo
   b) run_demos.bat
   c) Use 'imdisplay' to visually inspect all output files.

10)
   Distribution package is available as

     VisualMagick\bin\ImageMagick-1.0-Q8-dll.exe

