# Microsoft Developer Studio Project File - Name="fluid_wdll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=fluid_wdll - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fluid_wdll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fluid_wdll.mak" CFG="fluid_wdll - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fluid_wdll - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "fluid_wdll - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "fluid_wdll - Win32 Release MinSize" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fluid_wdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fluid___"
# PROP BASE Intermediate_Dir "fluid___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "fluid_wdll"
# PROP Intermediate_Dir "fluid_wdll"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "FL_SHARED" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 fltk2dll.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /machine:I386 /nodefaultlib:"libc" /out:"../../fluid/fluid2_wdll.exe" /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fluid_wdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fluid__0"
# PROP BASE Intermediate_Dir "fluid__0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "fluid_wdlld"
# PROP Intermediate_Dir "fluid_wdlld"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../visualc" /I "." /I "../.." /I "../../fltk/compat" /D "FL_SHARED" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 fltk2dlld.lib fltk2dll_imagesd.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd" /out:"../../fluid/fluid_wdlld.exe" /pdbtype:sept /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "fluid_wdll - Win32 Release MinSize"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fluid_wdll___Win32_Release_MinSize"
# PROP BASE Intermediate_Dir "fluid_wdll___Win32_Release_MinSize"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "fluid_wdll___Win32_Release_MinSize"
# PROP Intermediate_Dir "fluid_wdll___Win32_Release_MinSize"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "FL_SHARED" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /O1 /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "FL_SHARED" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /D "_MSC_DLL" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 fltk2dll.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /machine:I386 /nodefaultlib:"libc" /out:"../../fluid/fluid2_wdll.exe" /libpath:"..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 fltk2dll.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /machine:I386 /nodefaultlib:"libc" /out:"../../fluid/fluid2_wdll.exe" /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "fluid_wdll - Win32 Release"
# Name "fluid_wdll - Win32 Debug"
# Name "fluid_wdll - Win32 Release MinSize"
# Begin Source File

SOURCE=..\..\fluid\about_panel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\align_widget.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\alignment_panel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\code.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\CodeEditor.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\coding_style.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\coding_style_func.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\Enumeration.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\factory.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\file.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\fluid.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\Fluid_Image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\fluid_img.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\fluid_menus.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\Fluid_Plugins.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\FluidType.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\function_panel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\FunctionType.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\GroupType.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\MenuType.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\template_panel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\undo.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\widget_panel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\WidgetClassType.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\WidgetType.cxx
# End Source File
# Begin Source File

SOURCE=..\..\fluid\WindowType.cxx
# End Source File
# End Target
# End Project
