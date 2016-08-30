# Microsoft Developer Studio Project File - Name="libtiff" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libtiff - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libtiff.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libtiff.mak" CFG="libtiff - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libtiff - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libtiff - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "libtiff"
# PROP Scc_LocalPath "..\.."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libtiff - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I ".." /I "..\..\..\libtiff\contrib\winnt" /I "..\..\..\libtiff\libtiff" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "libtiff - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "." /I ".." /I "..\..\..\libtiff\contrib\winnt" /I "..\..\..\libtiff\libtiff" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FAs /FR /YX /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\dlibtiff.lib"

!ENDIF 

# Begin Target

# Name "libtiff - Win32 Release"
# Name "libtiff - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\..\..\libtiff\contrib\winnt\fax3sm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\libtiff\contrib\winnt\libtiff.def
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_aux.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_close.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_codec.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_compress.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_dir.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_dirinfo.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_dirread.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_dirwrite.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_dumpmode.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_error.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_fax3.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_flush.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_getimage.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_jpeg.c"
# End Source File
# Begin Source File

SOURCE=..\..\..\LibTiff\libtiff\tif_luv.c
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_lzw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_next.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_open.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_packbits.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_predict.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_print.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_read.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_strip.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_swab.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_thunder.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_tile.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_version.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_warning.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_win32.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_write.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\libtiff\libtiff\tif_zip.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\..\..\libtiff\libtiff\tif_fax3.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
