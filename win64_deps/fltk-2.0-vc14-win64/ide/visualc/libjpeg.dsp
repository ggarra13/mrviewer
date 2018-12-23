# Microsoft Developer Studio Project File - Name="libjpeg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libjpeg - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libjpeg.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libjpeg.mak" CFG="libjpeg - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libjpeg - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libjpeg - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "libjpeg - Win32 Release MinSize" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libjpeg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\fltk2_jpeg.lib"

!ELSEIF  "$(CFG)" == "libjpeg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "libjpeg___Win32_Debug"
# PROP BASE Intermediate_Dir "libjpeg___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../visualc" /I "." /I "../.." /I "../../fltk/compat" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /D "_LIB" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\fltk2_jpegd.lib"

!ELSEIF  "$(CFG)" == "libjpeg - Win32 Release MinSize"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "libjpeg___Win32_Release_MinSize"
# PROP BASE Intermediate_Dir "libjpeg___Win32_Release_MinSize"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libjpeg___Win32_Release_MinSize"
# PROP Intermediate_Dir "libjpeg___Win32_Release_MinSize"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /D "_LIB" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W1 /GX- /O1 /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /D "_LIB" /D "_MSC_DLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\fltk2_jpeg.lib"
# ADD LIB32 /nologo /out:"..\..\lib\fltk2_jpeg.lib"

!ENDIF 

# Begin Target

# Name "libjpeg - Win32 Release"
# Name "libjpeg - Win32 Debug"
# Name "libjpeg - Win32 Release MinSize"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\images\libjpeg\jcapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jccoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jccolor.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jchuff.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcinit.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcomapi.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcparam.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcprepct.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jcsample.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jctrans.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdatadst.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdcolor.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdhuff.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdinput.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdmerge.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdpostct.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdsample.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdtrans.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jerror.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jfdctint.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jidctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jidctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jidctint.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jidctred.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jmemnobs.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jquant1.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jquant2.c
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jutils.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\images\libjpeg\jchuff.h
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jconfig.h
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdct.h
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jdhuff.h
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jerror.h
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jinclude.h
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jmemsys.h
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jpegint.h
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jpeglib.h
# End Source File
# Begin Source File

SOURCE=..\..\images\libjpeg\jversion.h
# End Source File
# End Group
# End Target
# End Project
