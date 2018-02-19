# Microsoft Developer Studio Project File - Name="fltk_images" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=fltk_images - Win32 Debug Cairo
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltk_images.lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltk_images.lib.mak" CFG="fltk_images - Win32 Debug Cairo"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltk_images - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fltk_images - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fltk_images - Win32 Release MinSize" (based on "Win32 (x86) Static Library")
!MESSAGE "fltk_images - Win32 Debug Cairo" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltk_images - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "fltk"
# PROP Intermediate_Dir "fltk"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Os /Ob2 /I "../../images/zlib" /I "../visualc" /I "." /I "../.." /I "../../fltk/compat" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\fltk2_images.lib"

!ELSEIF  "$(CFG)" == "fltk_images - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "fltkd"
# PROP Intermediate_Dir "fltkd"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../../images/zlib" /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\fltk2_imagesd.lib"

!ELSEIF  "$(CFG)" == "fltk_images - Win32 Release MinSize"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fltk_images___Win32_Release_MinSize"
# PROP BASE Intermediate_Dir "fltk_images___Win32_Release_MinSize"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "fltk_images___Win32_Release_MinSize"
# PROP Intermediate_Dir "fltk_images___Win32_Release_MinSize"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Os /Ob2 /I "../../images/zlib" /I "../visualc" /I "." /I "../.." /I "../../fltk/compat" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /O1 /Ob2 /I "../../images/zlib" /I "../visualc" /I "." /I "../.." /I "../../fltk/compat" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /D "_MSC_DLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\fltk2_images.lib"
# ADD LIB32 /nologo /out:"..\..\lib\fltk2_images.lib"

!ELSEIF  "$(CFG)" == "fltk_images - Win32 Debug Cairo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fltk_images___Win32_Debug_Cairo"
# PROP BASE Intermediate_Dir "fltk_images___Win32_Debug_Cairo"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "fltk_images___Win32_Debug_Cairo"
# PROP Intermediate_Dir "fltk_images___Win32_Debug_Cairo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../../images/zlib" /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../../images/zlib" /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D USE_CAIRO=1 /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\fltk2_imagesd.lib"
# ADD LIB32 /nologo /out:"..\..\lib\fltk2_imagesd.lib"

!ENDIF 

# Begin Target

# Name "fltk_images - Win32 Release"
# Name "fltk_images - Win32 Debug"
# Name "fltk_images - Win32 Release MinSize"
# Name "fltk_images - Win32 Debug Cairo"
# Begin Group "headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\..\fltk\pnmImage.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\images\FileIcon2.cxx
# End Source File
# Begin Source File

SOURCE=..\..\images\Fl_Guess_Image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\images\fl_jpeg.cxx
# End Source File
# Begin Source File

SOURCE=..\..\images\fl_png.cxx
# End Source File
# Begin Source File

SOURCE=..\..\images\HelpDialog.cxx
# End Source File
# Begin Source File

SOURCE=..\..\images\images_core.cxx
# End Source File
# Begin Source File

SOURCE=..\..\images\pnmImage.cxx
# End Source File
# Begin Source File

SOURCE=..\..\images\xpmFileImage.cxx
# End Source File
# End Target
# End Project
