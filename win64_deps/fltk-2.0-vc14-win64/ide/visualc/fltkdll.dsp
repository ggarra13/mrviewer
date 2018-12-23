# Microsoft Developer Studio Project File - Name="fltkdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=fltkdll - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltkdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltkdll.mak" CFG="fltkdll - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltkdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fltkdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fltkdll - Win32 Release MinSize" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltkdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fltkdll0"
# PROP BASE Intermediate_Dir "fltkdll0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\lib\"
# PROP Intermediate_Dir "fltkdll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "FL_SHARED" /D "FL_LIBRARY" /D "FL_GL_LIBRARY" /D "FL_GLUT_LIBRARY" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 opengl32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:2.0 /subsystem:windows /dll /pdb:"fltkdll.pdb" /machine:I386 /nodefaultlib:"libc" /out:"..\..\lib\fltk2dll.dll" /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fltkdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fltkdll1"
# PROP BASE Intermediate_Dir "fltkdll1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\"
# PROP Intermediate_Dir "fltkdlld"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "_DEBUG" /D "FL_SHARED" /D "FL_LIBRARY" /D "FL_GL_LIBRARY" /D "FL_GLUT_LIBRARY" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:2.0 /subsystem:windows /dll /pdb:"fltkdlld.pdb" /debug /machine:I386 /nodefaultlib:"libcd" /out:"..\..\lib\fltk2dlld.dll" /pdbtype:sept /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "fltkdll - Win32 Release MinSize"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fltkdll___Win32_Release_MinSize"
# PROP BASE Intermediate_Dir "fltkdll___Win32_Release_MinSize"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\lib\"
# PROP Intermediate_Dir "fltkdll___Win32_Release_MinSize"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "FL_SHARED" /D "FL_LIBRARY" /D "FL_GL_LIBRARY" /D "FL_GLUT_LIBRARY" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /O1 /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "FL_SHARED" /D "FL_LIBRARY" /D "FL_GL_LIBRARY" /D "FL_GLUT_LIBRARY" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /D "_MSC_DLL" /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 opengl32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:2.0 /subsystem:windows /dll /pdb:"fltkdll.pdb" /machine:I386 /nodefaultlib:"libc" /out:"..\..\lib\fltk2dll.dll" /libpath:"..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 opengl32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:2.0 /subsystem:windows /dll /pdb:"fltkdll.pdb" /machine:I386 /nodefaultlib:"libc" /out:"..\..\lib\fltk2dll.dll" /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "fltkdll - Win32 Release"
# Name "fltkdll - Win32 Debug"
# Name "fltkdll - Win32 Release MinSize"
# Begin Source File

SOURCE=..\..\src\add_idle.cxx
DEP_CPP_ADD_I=\
	"..\..\fltk\run.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\addarc.cxx
DEP_CPP_ADDAR=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\addcurve.cxx
DEP_CPP_ADDCU=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Adjuster.cxx
DEP_CPP_ADJUS=\
	"..\..\fltk\adjuster.h"\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\src\fastarrow.h"\
	"..\..\src\mediumarrow.h"\
	"..\..\src\slowarrow.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\xbmimage.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\AlignGroup.cxx
DEP_CPP_ALIGN=\
	"..\..\fltk\aligngroup.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\args.cxx
DEP_CPP_ARGS_=\
	"..\..\fltk\error.h"\
	"..\..\fltk\filename.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\Monitor.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\array.h
# End Source File
# Begin Source File

SOURCE=..\..\src\BarGroup.cxx
DEP_CPP_BARGR=\
	"..\..\fltk\bargroup.h"\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\bmpImage.cxx
DEP_CPP_BMPIM=\
	"..\..\fltk\error.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\sharedimage.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Browser.cxx
DEP_CPP_BROWS=\
	"..\..\fltk\box.h"\
	"..\..\fltk\browser.h"\
	"..\..\fltk\cursor.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\error.h"\
	"..\..\fltk\layout.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\font.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\itemgroup.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Button.cxx
DEP_CPP_BUTTO=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\CheckButton.cxx
DEP_CPP_CHECK=\
	"..\..\fltk\box.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\checkbutton.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Choice.cxx
DEP_CPP_CHOIC=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	{$(INCLUDE)}"fltk\choice.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\clip.cxx
DEP_CPP_CLIP_=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\x.h"\
	"..\..\src\osx\clip.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Clock.cxx
DEP_CPP_CLOCK=\
	"..\..\fltk\clock.h"\
	"..\..\fltk\damage.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Color.cxx
DEP_CPP_COLOR=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\color_chooser.cxx
DEP_CPP_COLOR_=\
	"..\..\fltk\ask.h"\
	"..\..\fltk\box.h"\
	"..\..\fltk\colorchooser.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\layout.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\returnbutton.h"\
	"..\..\fltk\valueinput.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\floatinput.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\numericinput.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\popupmenu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\colormap.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ComboBox.cxx
DEP_CPP_COMBO=\
	"..\..\fltk\box.h"\
	"..\..\fltk\ComboBox.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\string.h"\
	{$(INCLUDE)}"fltk\choice.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\compose.cxx
DEP_CPP_COMPO=\
	".\config.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Cursor.cxx
DEP_CPP_CURSO=\
	"..\..\fltk\cursor.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\CycleButton.cxx
DEP_CPP_CYCLE=\
	"..\..\fltk\box.h"\
	"..\..\fltk\cyclebutton.h"\
	"..\..\fltk\damage.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\default_glyph.cxx
DEP_CPP_DEFAU=\
	"..\..\fltk\box.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Dial.cxx
DEP_CPP_DIAL_=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\dial.h"\
	"..\..\fltk\math.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\DiamondBox.cxx
DEP_CPP_DIAMO=\
	"..\..\fltk\box.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\dnd.cxx
DEP_CPP_DND_C=\
	"..\..\fltk\cursor.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\x.h"\
	"..\..\src\osx\dnd.cxx"\
	"..\..\src\win32\dnd.cxx"\
	"..\..\src\x11\dnd.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\drawtext.cxx
DEP_CPP_DRAWT=\
	"..\..\fltk\math.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\font.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\dump_compose.c
# End Source File
# Begin Source File

SOURCE=..\..\src\EngravedLabel.cxx
DEP_CPP_ENGRA=\
	"..\..\fltk\labeltype.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\error.cxx
DEP_CPP_ERROR=\
	"..\..\fltk\error.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\utf.h"\
	"..\..\fltk\x.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\event_key_state.cxx
DEP_CPP_EVENT=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\x.h"\
	"..\..\src\osx\event_key_state.cxx"\
	"..\..\src\win32\event_key_state.cxx"\
	"..\..\src\x11\event_key_state.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fastarrow.h
# End Source File
# Begin Source File

SOURCE=..\..\src\file_chooser.cxx
DEP_CPP_FILE_=\
	"..\..\fltk\ask.h"\
	"..\..\fltk\browser.h"\
	"..\..\fltk\DoubleBufferWindow.h"\
	"..\..\fltk\file_chooser.h"\
	"..\..\fltk\filebrowser.h"\
	"..\..\fltk\filechooser.h"\
	"..\..\fltk\fileicon.h"\
	"..\..\fltk\fileinput.h"\
	"..\..\fltk\filename.h"\
	"..\..\fltk\Preferences.h"\
	"..\..\fltk\returnbutton.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\tiledgroup.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\checkbutton.h"\
	{$(INCLUDE)}"fltk\choice.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\invisiblebox.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\popupmenu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\FileBrowser.cxx
DEP_CPP_FILEB=\
	"..\..\fltk\browser.h"\
	"..\..\fltk\filebrowser.h"\
	"..\..\fltk\fileicon.h"\
	"..\..\fltk\filename.h"\
	"..\..\fltk\string.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\font.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\FileChooser.cxx
DEP_CPP_FILEC=\
	"..\..\fltk\ask.h"\
	"..\..\fltk\browser.h"\
	"..\..\fltk\cursor.h"\
	"..\..\fltk\DoubleBufferWindow.h"\
	"..\..\fltk\filebrowser.h"\
	"..\..\fltk\filechooser.h"\
	"..\..\fltk\fileicon.h"\
	"..\..\fltk\fileinput.h"\
	"..\..\fltk\filename.h"\
	"..\..\fltk\Preferences.h"\
	"..\..\fltk\returnbutton.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\tiledgroup.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\checkbutton.h"\
	{$(INCLUDE)}"fltk\choice.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\invisiblebox.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\popupmenu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\xbmimage.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\FileChooser2.cxx
DEP_CPP_FILECH=\
	"..\..\fltk\ask.h"\
	"..\..\fltk\browser.h"\
	"..\..\fltk\cursor.h"\
	"..\..\fltk\DoubleBufferWindow.h"\
	"..\..\fltk\filebrowser.h"\
	"..\..\fltk\filechooser.h"\
	"..\..\fltk\fileicon.h"\
	"..\..\fltk\fileinput.h"\
	"..\..\fltk\filename.h"\
	"..\..\fltk\Preferences.h"\
	"..\..\fltk\returnbutton.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\tiledgroup.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\checkbutton.h"\
	{$(INCLUDE)}"fltk\choice.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\invisiblebox.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\popupmenu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\sharedimage.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\FileIcon.cxx
DEP_CPP_FILEI=\
	"..\..\fltk\box.h"\
	"..\..\fltk\browser.h"\
	"..\..\fltk\fileicon.h"\
	"..\..\fltk\filename.h"\
	"..\..\fltk\string.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\FileInput.cxx
DEP_CPP_FILEIN=\
	"..\..\fltk\box.h"\
	"..\..\fltk\cursor.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\fileinput.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\string.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_absolute.cxx
DEP_CPP_FILEN=\
	"..\..\fltk\filename.h"\
	"..\..\fltk\string.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_ext.cxx
DEP_CPP_FILENA=\
	"..\..\fltk\filename.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_isdir.cxx
DEP_CPP_FILENAM=\
	"..\..\fltk\filename.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\utf.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_list.cxx
DEP_CPP_FILENAME=\
	"..\..\fltk\filename.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\utf.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_match.cxx
DEP_CPP_FILENAME_=\
	"..\..\fltk\filename.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_name.cxx
DEP_CPP_FILENAME_N=\
	"..\..\fltk\filename.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fillrect.cxx
DEP_CPP_FILLR=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_Item.cxx
DEP_CPP_FL_ME=\
	"..\..\fltk\compat\fl\fl_menu_item.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\divider.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\itemgroup.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\popupmenu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\FloatInput.cxx
DEP_CPP_FLOAT=\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\floatinput.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\numericinput.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\fltk_theme.cxx
DEP_CPP_FLTK_=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\Monitor.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\utf.h"\
	"..\..\fltk\x.h"\
	"..\..\src\win32\fltk_theme.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\font.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Font.cxx
DEP_CPP_FONT_=\
	"..\..\fltk\error.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\utf.h"\
	"..\..\fltk\x.h"\
	"..\..\src\osx\Font.cxx"\
	"..\..\src\win32\Font.cxx"\
	"..\..\src\x11\Font.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\font.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	{$(INCLUDE)}"src\osx\utf8tomac.cxx"\
	{$(INCLUDE)}"src\x11\Font_xft.cxx"\
	{$(INCLUDE)}"src\x11\Font_xlfd.cxx"\
	{$(INCLUDE)}"src\x11\IFont.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\gifImage.cxx
DEP_CPP_GIFIM=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\sharedimage.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Group.cxx
DEP_CPP_GROUP=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\layout.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\GSave.cxx
DEP_CPP_GSAVE=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\HelpView.cxx
DEP_CPP_HELPV=\
	"..\..\fltk\box.h"\
	"..\..\fltk\cursor.h"\
	"..\..\fltk\HelpView.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\xpmimage.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\font.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\sharedimage.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\HighlightButton.cxx
DEP_CPP_HIGHL=\
	"..\..\fltk\highlightbutton.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Image.cxx
DEP_CPP_IMAGE=\
	"..\..\fltk\error.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\x.h"\
	"..\..\src\osx\Image.cxx"\
	"..\..\src\win32\Image.cxx"\
	"..\..\src\x11\Image.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	{$(INCLUDE)}"src\x11\XColorMap.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Input.cxx
DEP_CPP_INPUT=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\utf.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\InputBrowser.cxx
DEP_CPP_INPUTB=\
	"..\..\fltk\box.h"\
	"..\..\fltk\browser.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\inputbrowser.h"\
	"..\..\fltk\Monitor.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\menuwindow.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\InvisibleWidget.cxx
DEP_CPP_INVIS=\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\invisiblebox.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Item.cxx
DEP_CPP_ITEM_=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\checkbutton.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\divider.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\itemgroup.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\key_name.cxx
DEP_CPP_KEY_N=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\LightButton.cxx
DEP_CPP_LIGHT=\
	"..\..\fltk\box.h"\
	"..\..\fltk\lightbutton.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\checkbutton.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\list_fonts.cxx
DEP_CPP_LIST_=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\utf.h"\
	"..\..\fltk\x.h"\
	"..\..\src\osx\list_fonts.cxx"\
	"..\..\src\win32\list_fonts.cxx"\
	"..\..\src\x11\list_fonts.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\font.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	{$(INCLUDE)}"src\x11\IFont.h"\
	{$(INCLUDE)}"src\x11\list_fonts_xlfd.cxx"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\load_plugin.cxx
DEP_CPP_LOAD_=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\load_plugin.h"\
	"..\..\fltk\utf.h"\
	"..\..\fltk\x.h"\
	"..\..\src\dlload_osx.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\lock.cxx
DEP_CPP_LOCK_=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\Threads.h"\
	"..\..\fltk\x.h"\
	"..\..\src\win32\lock.cxx"\
	"..\..\src\x11\lock.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\mediumarrow.h
# End Source File
# Begin Source File

SOURCE=..\..\src\Menu.cxx
DEP_CPP_MENU_=\
	"..\..\fltk\damage.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Menu_add.cxx
DEP_CPP_MENU_A=\
	"..\..\fltk\string.h"\
	"..\..\src\array.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\divider.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\itemgroup.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Menu_global.cxx
DEP_CPP_MENU_G=\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Menu_popup.cxx
DEP_CPP_MENU_P=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\labeltype.h"\
	"..\..\fltk\Monitor.h"\
	"..\..\fltk\run.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\menuwindow.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\MenuBar.cxx
DEP_CPP_MENUB=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\tooltip.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\menubar.h"\
	{$(INCLUDE)}"fltk\menuwindow.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\MenuWindow.cxx
DEP_CPP_MENUW=\
	"..\..\fltk\damage.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\menuwindow.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\message.cxx
DEP_CPP_MESSA=\
	"..\..\fltk\ask.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\returnbutton.h"\
	"..\..\fltk\secretinput.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\x.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\MultiImage.cxx
DEP_CPP_MULTI=\
	"..\..\fltk\multiimage.h"\
	"..\..\fltk\string.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\NumericInput.cxx
DEP_CPP_NUMER=\
	"..\..\fltk\math.h"\
	"..\..\fltk\string.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\numericinput.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\numericsort.cxx
DEP_CPP_NUMERI=\
	"..\..\fltk\filename.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Output.cxx
DEP_CPP_OUTPU=\
	"..\..\fltk\output.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\OvalBox.cxx
DEP_CPP_OVALB=\
	"..\..\fltk\box.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\overlay_rect.cxx
DEP_CPP_OVERL=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\own_colormap.cxx
DEP_CPP_OWN_C=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\visual.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\PackedGroup.cxx
DEP_CPP_PACKE=\
	"..\..\fltk\box.h"\
	"..\..\fltk\layout.h"\
	"..\..\fltk\packedgroup.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\path.cxx
DEP_CPP_PATH_=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\PlasticBox.cxx
DEP_CPP_PLAST=\
	"..\..\fltk\box.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\PopupMenu.cxx
DEP_CPP_POPUP=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\popupmenu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Preferences.cxx
DEP_CPP_PREFE=\
	"..\..\fltk\filename.h"\
	"..\..\fltk\Preferences.h"\
	"..\..\fltk\string.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\ProgressBar.cxx
DEP_CPP_PROGR=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\progressbar.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\RadioButton.cxx
DEP_CPP_RADIO=\
	"..\..\fltk\box.h"\
	"..\..\fltk\radiobutton.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\checkbutton.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\readimage.cxx
DEP_CPP_READI=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\x.h"\
	"..\..\src\osx\readimage.cxx"\
	"..\..\src\win32\readimage.cxx"\
	"..\..\src\x11\readimage.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\RepeatButton.cxx
DEP_CPP_REPEA=\
	"..\..\fltk\repeatbutton.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\ReturnButton.cxx
DEP_CPP_RETUR=\
	"..\..\fltk\returnbutton.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\RoundBox.cxx
DEP_CPP_ROUND=\
	"..\..\fltk\box.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\RoundedBox.cxx
DEP_CPP_ROUNDE=\
	"..\..\fltk\box.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\run.cxx
DEP_CPP_RUN_C=\
	"..\..\fltk\browser.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\error.h"\
	"..\..\fltk\filename.h"\
	"..\..\fltk\FL_VERSION.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\layout.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\Monitor.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\SystemMenuBar.h"\
	"..\..\fltk\tooltip.h"\
	"..\..\fltk\utf.h"\
	"..\..\fltk\visual.h"\
	"..\..\fltk\x.h"\
	"..\..\src\osx\run.cxx"\
	"..\..\src\win32\run.cxx"\
	"..\..\src\x11\run.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\font.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\itemgroup.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\menubar.h"\
	{$(INCLUDE)}"fltk\menuwindow.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	{$(INCLUDE)}"src\osx\mactoutf8.cxx"\
	{$(INCLUDE)}"src\win32\pktdef.h"\
	{$(INCLUDE)}"src\win32\wintab.h"\
	{$(INCLUDE)}"src\x11\xutf8.cxx"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\scandir.c
DEP_CPP_SCAND=\
	"..\..\fltk\utf.h"\
	"..\..\src\win32\scandir.c"\
	".\config.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Scrollbar.cxx
DEP_CPP_SCROL=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\ScrollGroup.cxx
DEP_CPP_SCROLL=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\layout.h"\
	"..\..\fltk\scrollgroup.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\scrollrect.cxx
DEP_CPP_SCROLLR=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\setcolor.cxx
DEP_CPP_SETCO=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\x.h"\
	"..\..\src\colormap.h"\
	"..\..\src\osx\setcolor.cxx"\
	"..\..\src\win32\setcolor.cxx"\
	"..\..\src\x11\setcolor.cxx"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	{$(INCLUDE)}"src\x11\XColorMap.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\setdisplay.cxx
DEP_CPP_SETDI=\
	"..\..\fltk\run.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\setvisual.cxx
DEP_CPP_SETVI=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\visual.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\ShadowBox.cxx
DEP_CPP_SHADO=\
	"..\..\fltk\box.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\ShapedWindow.cxx
DEP_CPP_SHAPE=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\shapedwindow.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	{$(INCLUDE)}"fltk\xbmimage.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\SharedImage.cxx
DEP_CPP_SHARE=\
	"..\..\fltk\string.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\sharedimage.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\xbmimage.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\ShortcutAssignment.cxx
DEP_CPP_SHORT=\
	"..\..\fltk\string.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\show_colormap.cxx
DEP_CPP_SHOW_=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\Monitor.h"\
	"..\..\fltk\show_colormap.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\menuwindow.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Slider.cxx
DEP_CPP_SLIDE=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\math.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\slowarrow.h
# End Source File
# Begin Source File

SOURCE=..\..\src\StatusBarGroup.cxx
DEP_CPP_STATU=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\layout.h"\
	"..\..\fltk\StatusBarGroup.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\invisiblebox.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\string.c
DEP_CPP_STRIN=\
	"..\..\fltk\string.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\StringList.cxx
DEP_CPP_STRING=\
	"..\..\fltk\stringlist.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Style.cxx
DEP_CPP_STYLE=\
	"..\..\fltk\math.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\string.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\StyleSet.cxx
DEP_CPP_STYLES=\
	"..\..\fltk\styleset.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Symbol.cxx
DEP_CPP_SYMBO=\
	"..\..\fltk\error.h"\
	"..\..\fltk\math.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\TabGroup.cxx
DEP_CPP_TABGR=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\layout.h"\
	"..\..\fltk\tabgroup.h"\
	"..\..\fltk\tooltip.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\menuwindow.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\TabGroup2.cxx
DEP_CPP_TABGRO=\
	"..\..\fltk\ask.h"\
	"..\..\fltk\box.h"\
	"..\..\fltk\MenuBuild.h"\
	"..\..\fltk\tabgroup.h"\
	{$(INCLUDE)}"fltk\choice.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\divider.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\item.h"\
	{$(INCLUDE)}"fltk\itemgroup.h"\
	{$(INCLUDE)}"fltk\menu.h"\
	{$(INCLUDE)}"fltk\menubar.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\popupmenu.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\TextBuffer.cxx
DEP_CPP_TEXTB=\
	"..\..\fltk\ask.h"\
	"..\..\fltk\error.h"\
	"..\..\fltk\string.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\textbuffer.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\TextDisplay.cxx
DEP_CPP_TEXTD=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\error.h"\
	"..\..\fltk\layout.h"\
	"..\..\fltk\utf.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\font.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\textbuffer.h"\
	{$(INCLUDE)}"fltk\textdisplay.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\TextEditor.cxx
DEP_CPP_TEXTE=\
	"..\..\fltk\ask.h"\
	"..\..\fltk\texteditor.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\font.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\scrollbar.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\textbuffer.h"\
	{$(INCLUDE)}"fltk\textdisplay.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\ThumbWheel.cxx
DEP_CPP_THUMB=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\thumbwheel.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\TiledGroup.cxx
DEP_CPP_TILED=\
	"..\..\fltk\cursor.h"\
	"..\..\fltk\layout.h"\
	"..\..\fltk\tiledgroup.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\TiledImage.cxx
DEP_CPP_TILEDI=\
	"..\..\fltk\tiledimage.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Tooltip.cxx
DEP_CPP_TOOLT=\
	"..\..\fltk\box.h"\
	"..\..\fltk\Monitor.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\tooltip.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\menuwindow.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\UpBox.cxx
DEP_CPP_UPBOX=\
	"..\..\fltk\box.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\utf.c
DEP_CPP_UTF_C=\
	"..\..\fltk\utf.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Valuator.cxx
DEP_CPP_VALUA=\
	"..\..\fltk\damage.h"\
	"..\..\fltk\math.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\ValueInput.cxx
DEP_CPP_VALUE=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\valueinput.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\floatinput.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\numericinput.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\ValueOutput.cxx
DEP_CPP_VALUEO=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\valueoutput.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\ValueSlider.cxx
DEP_CPP_VALUES=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\math.h"\
	"..\..\fltk\valueslider.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\floatinput.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\input.h"\
	{$(INCLUDE)}"fltk\numericinput.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\slider.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\valuator.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\vsnprintf.c
DEP_CPP_VSNPR=\
	"..\..\fltk\string.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Widget.cxx
DEP_CPP_WIDGE=\
	"..\..\fltk\damage.h"\
	"..\..\fltk\layout.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\string.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Widget_draw.cxx
DEP_CPP_WIDGET=\
	"..\..\fltk\box.h"\
	"..\..\fltk\damage.h"\
	"..\..\fltk\labeltype.h"\
	"..\..\fltk\string.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Window.cxx
DEP_CPP_WINDO=\
	"..\..\fltk\damage.h"\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\layout.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Window_fullscreen.cxx
DEP_CPP_WINDOW=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\Monitor.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Window_hotspot.cxx
DEP_CPP_WINDOW_=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\Monitor.h"\
	"..\..\fltk\x.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\events.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\Window_iconize.cxx
DEP_CPP_WINDOW_I=\
	"..\..\fltk\fltk_cairo.h"\
	"..\..\fltk\x.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\osx.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"fltk\window.h"\
	{$(INCLUDE)}"fltk\x11.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\WinMain.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\WizardGroup.cxx
DEP_CPP_WIZAR=\
	"..\..\fltk\cursor.h"\
	"..\..\fltk\WizardGroup.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\style.h"\
	{$(INCLUDE)}"fltk\widget.h"\
	{$(INCLUDE)}"fltk\window.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\xbmImage.cxx
DEP_CPP_XBMIM=\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\xbmimage.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\src\xpmImage.cxx
DEP_CPP_XPMIM=\
	"..\..\fltk\string.h"\
	"..\..\fltk\xpmimage.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# End Target
# End Project
