# Microsoft Developer Studio Project File - Name="fltkdll_images" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=fltkdll_images - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltkdll_images.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltkdll_images.mak" CFG="fltkdll_images - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltkdll_images - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fltkdll_images - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fltkdll_images - Win32 Release MinSize" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltkdll_images - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fltkdll_images0"
# PROP BASE Intermediate_Dir "fltkdll_images0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\lib\"
# PROP Intermediate_Dir "fltkdll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /I "../../images/zlib" /D "NDEBUG" /D "FL_SHARED" /D "FL_IMAGES_LIBRARY" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /c
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
# ADD LINK32 fltk2dll.lib fltk2_z.lib fltk2_png.lib fltk2_jpeg.lib opengl32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltkdll_images.pdb" /machine:I386 /nodefaultlib:"libc" /out:"..\..\lib\fltk2dll_images.dll" /libpath:"../../lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fltkdll_images - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fltkdll_images1"
# PROP BASE Intermediate_Dir "fltkdll_images1"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\lib\"
# PROP Intermediate_Dir "fltkdlld"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../../images/zlib" /I "../visualc" /I "." /I "../.." /I "../../fltk/compat" /D "_DEBUG" /D "FL_SHARED" /D "FL_IMAGES_LIBRARY" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /c
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
# ADD LINK32 fltk2dlld.lib fltk2_zd.lib fltk2_pngd.lib fltk2_jpegd.lib opengl32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltkdll_imagesd.pdb" /debug /machine:I386 /nodefaultlib:"libcd" /out:"..\..\lib\fltk2dll_imagesd.dll" /pdbtype:sept /libpath:"../../lib"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "fltkdll_images - Win32 Release MinSize"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fltkdll_images___Win32_Release_MinSize"
# PROP BASE Intermediate_Dir "fltkdll_images___Win32_Release_MinSize"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\lib\"
# PROP Intermediate_Dir "fltkdll_images___Win32_Release_MinSize"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /I "../../images/zlib" /D "NDEBUG" /D "FL_SHARED" /D "FL_IMAGES_LIBRARY" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W1 /GX- /O1 /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /I "../../images/zlib" /D "NDEBUG" /D "FL_SHARED" /D "FL_IMAGES_LIBRARY" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /D "_MSC_DLL" /YX /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 fltk2dll.lib fltk2_z.lib fltk2_png.lib fltk2_jpeg.lib opengl32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltkdll_images.pdb" /machine:I386 /nodefaultlib:"libc" /out:"..\..\lib\fltk2dll_images.dll" /libpath:"../../lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 fltk2dll.lib fltk2_z.lib fltk2_png.lib fltk2_jpeg.lib opengl32.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /version:1.0 /subsystem:windows /dll /pdb:"fltkdll_images.pdb" /machine:I386 /nodefaultlib:"libc" /out:"..\..\lib\fltk2dll_images.dll" /libpath:"../../lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "fltkdll_images - Win32 Release"
# Name "fltkdll_images - Win32 Debug"
# Name "fltkdll_images - Win32 Release MinSize"
# Begin Source File

SOURCE=..\..\images\FileIcon2.cxx
DEP_CPP_FILEI=\
	"..\..\fltk\fileicon.h"\
	"..\..\fltk\filename.h"\
	"..\..\fltk\sharedimage.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\types.h"\
	"..\..\fltk\xpmimage.h"\
	"..\..\pixmaps\file_small2.xpm"\
	"..\..\pixmaps\folder_small2.xpm"\
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
# Begin Source File

SOURCE=..\..\images\Fl_Guess_Image.cxx
DEP_CPP_FL_GU=\
	"..\..\fltk\sharedimage.h"\
	"..\..\fltk\types.h"\
	"..\..\fltk\x.h"\
	"..\..\fltk\xbmimage.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\mac.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\images\fl_jpeg.cxx
DEP_CPP_FL_JP=\
	"..\..\fltk\sharedimage.h"\
	"..\..\fltk\types.h"\
	"..\..\images\libjpeg\jpeglib.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"images\libjpeg\jconfig.h"\
	{$(INCLUDE)}"images\libjpeg\jerror.h"\
	{$(INCLUDE)}"images\libjpeg\jmorecfg.h"\
	{$(INCLUDE)}"images\libjpeg\jpegint.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\images\fl_png.cxx
DEP_CPP_FL_PN=\
	"..\..\fltk\error.h"\
	"..\..\fltk\sharedimage.h"\
	"..\..\fltk\types.h"\
	"..\..\fltk\x.h"\
	"..\..\images\libpng\png.h"\
	"..\..\images\zlib\zlib.h"\
	".\config.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\mac.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	{$(INCLUDE)}"images\libpng\pngconf.h"\
	{$(INCLUDE)}"images\zlib\zconf.h"\
	
NODEP_CPP_FL_PN=\
	"..\..\images\libpng\alloc.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\images\HelpDialog.cxx
DEP_CPP_HELPD=\
	"..\..\fltk\ask.h"\
	"..\..\fltk\DoubleBufferWindow.h"\
	"..\..\fltk\HelpDialog.h"\
	"..\..\fltk\HelpView.h"\
	"..\..\fltk\sharedimage.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\types.h"\
	{$(INCLUDE)}"fltk\button.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\group.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\input.h"\
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

SOURCE=..\..\images\images_core.cxx
DEP_CPP_IMAGE=\
	"..\..\fltk\sharedimage.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\types.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\images\pnmImage.cxx
DEP_CPP_PNMIM=\
	"..\..\fltk\error.h"\
	"..\..\fltk\pnmImage.h"\
	"..\..\fltk\run.h"\
	"..\..\fltk\sharedimage.h"\
	"..\..\fltk\string.h"\
	"..\..\fltk\types.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	
# End Source File
# Begin Source File

SOURCE=..\..\images\xpmFileImage.cxx
DEP_CPP_XPMFI=\
	"..\..\fltk\events.h"\
	"..\..\fltk\sharedimage.h"\
	"..\..\fltk\types.h"\
	"..\..\fltk\x.h"\
	"..\..\fltk\xpmimage.h"\
	{$(INCLUDE)}"fltk\color.h"\
	{$(INCLUDE)}"fltk\draw.h"\
	{$(INCLUDE)}"fltk\fl_api.h"\
	{$(INCLUDE)}"fltk\flags.h"\
	{$(INCLUDE)}"fltk\image.h"\
	{$(INCLUDE)}"fltk\mac.h"\
	{$(INCLUDE)}"fltk\PixelType.h"\
	{$(INCLUDE)}"fltk\rectangle.h"\
	{$(INCLUDE)}"fltk\symbol.h"\
	{$(INCLUDE)}"fltk\win32.h"\
	
# End Source File
# End Target
# End Project
