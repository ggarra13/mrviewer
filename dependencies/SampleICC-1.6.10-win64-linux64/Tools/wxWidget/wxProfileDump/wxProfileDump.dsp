# Microsoft Developer Studio Project File - Name="wxProfileDump" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=wxProfileDump - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "wxProfileDump.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "wxProfileDump.mak" CFG="wxProfileDump - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wxProfileDump - Win32 DLL Universal Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 DLL Universal Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 DLL Universal Release" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 DLL Universal Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 DLL Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 DLL Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 DLL Release" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 DLL Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 Universal Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 Universal Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 Universal Release" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 Universal Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 Unicode Release" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 Unicode Debug" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "wxProfileDump - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "wxProfileDump"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vc_mswunivdll"
# PROP BASE Intermediate_Dir "vc_mswunivdll\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vc_mswunivdll"
# PROP Intermediate_Dir "vc_mswunivdll\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /Gm /GR /Zi /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\mswunivu" /I "." /I "$(WXWIN)\samples" /I "$(WXWIN)\lib\vc_dll\mswunivud" /I "$(WXWIN)\lib\vc_dll\mswuniv" /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /D "_DEBUG" /D "__WXDEBUG__" /Fd"vc_mswunivdll\mdi.pdb" /FD /EHsc /GZ /EHsc /EHsc /c
# ADD CPP /nologo /MD /W4 /Gm /GR /Zi /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\mswunivu" /I "." /I "$(WXWIN)\samples" /I "$(WXWIN)\lib\vc_dll\mswunivud" /I "$(WXWIN)\lib\vc_dll\mswuniv" /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /D "_DEBUG" /D "__WXDEBUG__" /Fd"vc_mswunivdll\mdi.pdb" /FD /EHsc /GZ /EHsc /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /D "_DEBUG" /D "__WXDEBUG__" /mktyplib203 /win32 /win32 /win32
# ADD MTL /nologo /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /D "_DEBUG" /D "__WXDEBUG__" /mktyplib203 /win32 /win32 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\mswunivu" /i "." /i "$(WXWIN)\samples" /i "$(WXWIN)\lib\vc_dll\mswunivud" /i "$(WXWIN)\lib\vc_dll\mswuniv" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "_UNICODE" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH" /d "_DEBUG" /d "__WXDEBUG__"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\mswunivu" /i "." /i "$(WXWIN)\samples" /i "$(WXWIN)\lib\vc_dll\mswunivud" /i "$(WXWIN)\lib\vc_dll\mswuniv" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "_UNICODE" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH" /d "_DEBUG" /d "__WXDEBUG__"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmswuniv26u_core.lib wxbase26u.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregexu.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib wxmswuniv26ud_core.lib wxbase26ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib wxmswuniv26_core.lib wxbase26.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"vc_mswunivdll\mdi.exe" /libpath:"$(WXWIN)\lib\vc_dll"
# ADD LINK32 wxmswuniv26u_core.lib wxbase26u.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregexu.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib wxmswuniv26ud_core.lib wxbase26ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib wxmswuniv26_core.lib wxbase26.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"vc_mswunivdll\mdi.exe" /libpath:"$(WXWIN)\lib\vc_dll"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Unicode Debug"

BSC32=bscmake.exe
LINK32=link.exe

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Release"

BSC32=bscmake.exe
LINK32=link.exe

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "vc_mswunivddll"
# PROP BASE Intermediate_Dir "vc_mswunivddll\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "vc_mswunivddll"
# PROP Intermediate_Dir "vc_mswunivddll\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\mswunivd" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswunivddll\mdi.pdb" /FD /GZ /EHsc /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\mswunivd" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswunivddll\mdi.pdb" /FD /GZ /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\mswunivd" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "__WXDEBUG__" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\mswunivd" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "__WXDEBUG__" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmswuniv26d_core.lib wxbase26d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"vc_mswunivddll\mdi.exe" /libpath:"$(WXWIN)\lib\vc_dll"
# ADD LINK32 wxmswuniv26d_core.lib wxbase26d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"vc_mswunivddll\mdi.exe" /libpath:"$(WXWIN)\lib\vc_dll"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vc_mswudll"
# PROP BASE Intermediate_Dir "vc_mswudll\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vc_mswudll"
# PROP Intermediate_Dir "vc_mswudll\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\mswu" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswudll\wxProfileDump.pdb" /FD /EHsc /c
# ADD CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\mswu" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswudll\wxProfileDump.pdb" /FD /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "__WXMSW__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "__WXMSW__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\mswu" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "_UNICODE" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\mswu" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "_UNICODE" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw26u_core.lib wxbase26u.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregexu.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_dll"
# ADD LINK32 wxmsw26u_core.lib wxbase26u.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregexu.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_dll"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "vc_mswuddll"
# PROP BASE Intermediate_Dir "vc_mswuddll\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "vc_mswuddll"
# PROP Intermediate_Dir "vc_mswuddll\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\mswud" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswuddll\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\mswud" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswuddll\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_UNICODE" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\mswud" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXDEBUG__" /d "_UNICODE" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\mswud" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXDEBUG__" /d "_UNICODE" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw26ud_core.lib wxbase26ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_dll"
# ADD LINK32 wxmsw26ud_core.lib wxbase26ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_dll"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vc_mswdll"
# PROP BASE Intermediate_Dir "vc_mswdll\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vc_mswdll"
# PROP Intermediate_Dir "vc_mswdll\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\msw" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswdll\wxProfileDump.pdb" /FD /EHsc /c
# ADD CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\msw" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswdll\wxProfileDump.pdb" /FD /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "__WXMSW__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "__WXMSW__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\msw" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\msw" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw26_core.lib wxbase26.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_dll"
# ADD LINK32 wxmsw26_core.lib wxbase26.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_dll"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "vc_mswddll"
# PROP BASE Intermediate_Dir "vc_mswddll\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "vc_mswddll"
# PROP Intermediate_Dir "vc_mswddll\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\mswd" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswddll\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_dll\mswd" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswddll\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "WXUSINGDLL" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\mswd" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXDEBUG__" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_dll\mswd" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXDEBUG__" /d "WXUSINGDLL" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw26d_core.lib wxbase26d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_dll"
# ADD LINK32 wxmsw26d_core.lib wxbase26d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_dll"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vc_mswunivu"
# PROP BASE Intermediate_Dir "vc_mswunivu\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vc_mswunivu"
# PROP Intermediate_Dir "vc_mswunivu\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswunivu" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswunivu\wxProfileDump.pdb" /FD /EHsc /c
# ADD CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswunivu" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswunivu\wxProfileDump.pdb" /FD /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswunivu" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "_UNICODE" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswunivu" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "_UNICODE" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmswuniv26u_core.lib wxbase26u.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregexu.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"
# ADD LINK32 wxmswuniv26u_core.lib wxbase26u.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregexu.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "vc_mswunivud"
# PROP BASE Intermediate_Dir "vc_mswunivud\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "vc_mswunivud"
# PROP Intermediate_Dir "vc_mswunivud\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswunivud" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswunivud\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswunivud" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswunivud\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswunivud" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "__WXDEBUG__" /d "_UNICODE" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswunivud" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "__WXDEBUG__" /d "_UNICODE" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmswuniv26ud_core.lib wxbase26ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"
# ADD LINK32 wxmswuniv26ud_core.lib wxbase26ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vc_mswuniv"
# PROP BASE Intermediate_Dir "vc_mswuniv\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vc_mswuniv"
# PROP Intermediate_Dir "vc_mswuniv\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswuniv" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswuniv\wxProfileDump.pdb" /FD /EHsc /c
# ADD CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswuniv" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswuniv\wxProfileDump.pdb" /FD /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswuniv" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswuniv" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmswuniv26_core.lib wxbase26.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"
# ADD LINK32 wxmswuniv26_core.lib wxbase26.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "vc_mswunivd"
# PROP BASE Intermediate_Dir "vc_mswunivd\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "vc_mswunivd"
# PROP Intermediate_Dir "vc_mswunivd\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswunivd" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswunivd\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswunivd" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswunivd\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXUNIVERSAL__" /D "__WXDEBUG__" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswunivd" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "__WXDEBUG__" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswunivd" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXUNIVERSAL__" /d "__WXDEBUG__" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmswuniv26d_core.lib wxbase26d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"
# ADD LINK32 wxmswuniv26d_core.lib wxbase26d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vc_mswu"
# PROP BASE Intermediate_Dir "vc_mswu\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vc_mswu"
# PROP Intermediate_Dir "vc_mswu\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswu" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswu\wxProfileDump.pdb" /FD /EHsc /c
# ADD CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswu" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswu\wxProfileDump.pdb" /FD /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "__WXMSW__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "__WXMSW__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswu" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "_UNICODE" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswu" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "_UNICODE" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw26u_core.lib wxbase26u.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregexu.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"
# ADD LINK32 wxmsw26u_core.lib wxbase26u.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregexu.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "vc_mswud"
# PROP BASE Intermediate_Dir "vc_mswud\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "vc_mswud"
# PROP Intermediate_Dir "vc_mswud\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswud" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswud\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswud" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswud\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_UNICODE" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswud" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXDEBUG__" /d "_UNICODE" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswud" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXDEBUG__" /d "_UNICODE" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw26ud_core.lib wxbase26ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"
# ADD LINK32 wxmsw26ud_core.lib wxbase26ud.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexud.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vc_msw"
# PROP BASE Intermediate_Dir "vc_msw\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "vc_msw"
# PROP Intermediate_Dir "vc_msw\wxProfileDump"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W4 /GR /O2 /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\msw" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "_WINDOWS" /D "NOPCH" /Fd"vc_msw\wxProfileDump.pdb" /FD /EHsc /c
# ADD CPP /nologo /MD /W4 /GR /O2 /I "..\..\..\IccProfLib" /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\msw" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "__WXMSW__" /D "_WINDOWS" /D "NOPCH" /Fd"vc_msw\wxProfileDump.pdb" /FD /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "__WXMSW__" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "__WXMSW__" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\msw" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\msw" /i "." /i "$(WXWIN)\samples" /d "__WXMSW__" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw26_core.lib wxbase26.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"
# ADD LINK32 wxmsw26_core.lib wxbase26.lib wxtiff.lib wxjpeg.lib wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /machine:I386 /libpath:"$(WXWINVC6)\lib\vc_lib" /libpath:"$(WXWIN)\lib\vc_lib"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "vc_mswd"
# PROP BASE Intermediate_Dir "vc_mswd\wxProfileDump"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "vc_mswd"
# PROP Intermediate_Dir "vc_mswd\wxProfileDump"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswd" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswd\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD CPP /nologo /MDd /W4 /Gm /GR /Zi /Od /I "..\..\..\IccProfLib" /I "$(WXWIN)\include" /I "$(WXWIN)\lib\vc_lib\mswd" /I "." /I "$(WXWIN)\samples" /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_WINDOWS" /D "NOPCH" /Fd"vc_mswd\wxProfileDump.pdb" /FD /GZ /EHsc /c
# ADD BASE MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD MTL /nologo /D "WIN32" /D "_DEBUG" /D "__WXMSW__" /D "__WXDEBUG__" /D "_WINDOWS" /D "NOPCH" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswd" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXDEBUG__" /d "_WINDOWS" /d "NOPCH"
# ADD RSC /l 0x409 /i "$(WXWIN)\include" /i "$(WXWIN)\lib\vc_lib\mswd" /i "." /i "$(WXWIN)\samples" /d "_DEBUG" /d "__WXMSW__" /d "__WXDEBUG__" /d "_WINDOWS" /d "NOPCH"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wxmsw26d_core.lib wxbase26d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWIN)\lib\vc_lib"
# ADD LINK32 wxmsw26d_core.lib wxbase26d.lib wxtiffd.lib wxjpegd.lib wxpngd.lib wxzlibd.lib wxregexd.lib wxexpatd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib winmm.lib shell32.lib comctl32.lib ole32.lib oleaut32.lib uuid.lib rpcrt4.lib advapi32.lib wsock32.lib oleacc.lib odbc32.lib /nologo /subsystem:windows /debug /machine:I386 /libpath:"$(WXWINVC6)\lib\vc_lib" /libpath:"$(WXWIN)\lib\vc_lib"

!ENDIF 

# Begin Target

# Name "wxProfileDump - Win32 DLL Universal Unicode Release"
# Name "wxProfileDump - Win32 DLL Universal Unicode Debug"
# Name "wxProfileDump - Win32 DLL Universal Release"
# Name "wxProfileDump - Win32 DLL Universal Debug"
# Name "wxProfileDump - Win32 DLL Unicode Release"
# Name "wxProfileDump - Win32 DLL Unicode Debug"
# Name "wxProfileDump - Win32 DLL Release"
# Name "wxProfileDump - Win32 DLL Debug"
# Name "wxProfileDump - Win32 Universal Unicode Release"
# Name "wxProfileDump - Win32 Universal Unicode Debug"
# Name "wxProfileDump - Win32 Universal Release"
# Name "wxProfileDump - Win32 Universal Debug"
# Name "wxProfileDump - Win32 Unicode Release"
# Name "wxProfileDump - Win32 Unicode Debug"
# Name "wxProfileDump - Win32 Release"
# Name "wxProfileDump - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\wxProfileDump.cpp

!IF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Unicode Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Unicode Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Unicode Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Unicode Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Unicode Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Unicode Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Unicode Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wxProfileDump.rc
# End Source File
# End Group
# Begin Group "Library Files"

# PROP Default_Filter "lib"
# Begin Source File

SOURCE=..\..\..\IccProfLib\Debug_CRTDLL\IccProfLib_CRTDLL.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\IccProfLib\Release_CRTDLL\IccProfLib_CRTDLL.lib

!IF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Unicode Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Unicode Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Universal Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Unicode Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Unicode Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 DLL Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Unicode Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Unicode Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Universal Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Unicode Release"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Unicode Debug"

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "wxProfileDump - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
