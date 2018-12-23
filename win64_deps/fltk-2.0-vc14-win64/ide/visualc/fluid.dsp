# Microsoft Developer Studio Project File - Name="fluid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=fluid - Win32 Debug Cairo
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fluid.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fluid.mak" CFG="fluid - Win32 Debug Cairo"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fluid - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "fluid - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "fluid - Win32 Release MinSize" (based on "Win32 (x86) Application")
!MESSAGE "fluid - Win32 Debug Cairo" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fluid - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fluid___"
# PROP BASE Intermediate_Dir "fluid___"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "fluid"
# PROP Intermediate_Dir "fluid"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
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
# ADD LINK32 fltk2.lib fltk2_images.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /machine:I386 /nodefaultlib:"libc" /out:"../../fluid/fluid2.exe" /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fluid - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fluid__0"
# PROP BASE Intermediate_Dir "fluid__0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "fluidd"
# PROP Intermediate_Dir "fluidd"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../visualc" /I "." /I "../.." /I "../../fltk/compat" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
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
# ADD LINK32 fltk2d.lib fltk2_imagesd.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /profile /debug /machine:I386 /nodefaultlib:"libcd" /out:"../../fluid/fluid2d.exe" /libpath:"..\..\lib"

!ELSEIF  "$(CFG)" == "fluid - Win32 Release MinSize"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fluid___Win32_Release_MinSize"
# PROP BASE Intermediate_Dir "fluid___Win32_Release_MinSize"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "fluid___Win32_Release_MinSize"
# PROP Intermediate_Dir "fluid___Win32_Release_MinSize"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /O1 /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /D "_MSC_DLL" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 fltk2.lib fltk2_images.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /machine:I386 /nodefaultlib:"libc" /out:"../../fluid/fluid2.exe" /libpath:"..\..\lib"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 fltk2.lib fltk2_images.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /machine:I386 /nodefaultlib:"libc" /out:"../../fluid/fluid2.exe" /libpath:"..\..\lib"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "fluid - Win32 Debug Cairo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fluid___Win32_Debug_Cairo"
# PROP BASE Intermediate_Dir "fluid___Win32_Debug_Cairo"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "fluid___Win32_Debug_Cairo"
# PROP Intermediate_Dir "fluid___Win32_Debug_Cairo"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../visualc" /I "." /I "../.." /I "../../fltk/compat" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GX /ZI /Od /I "../visualc" /I "." /I "../.." /I "../../fltk/compat" /D USE_CAIRO=1 /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 fltk2d.lib fltk2_imagesd.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /profile /debug /machine:I386 /nodefaultlib:"libcd" /out:"../../fluid/fluid2d.exe" /libpath:"..\..\lib"
# ADD LINK32 cairo.lib fltk2d.lib fltk2_imagesd.lib fltk2_jpegd.lib fltk2_zd.lib ws2_32.lib fltk2_pngd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib msimg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"mainCRTStartup" /subsystem:windows /profile /debug /machine:I386 /nodefaultlib:"libcd" /out:"../../fluid/fluid2d.exe" /libpath:"..\..\lib"

!ENDIF 

# Begin Target

# Name "fluid - Win32 Release"
# Name "fluid - Win32 Debug"
# Name "fluid - Win32 Release MinSize"
# Name "fluid - Win32 Debug Cairo"
# Begin Group "headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\..\fluid\about_panel.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\alignment_panel.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\CodeEditor.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\coding_style.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\factory.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\fluid.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\Fluid_Image.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\fluid_img.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\fluid_menus.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\Fluid_Plugins.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\FluidType.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\function_panel.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\FunctionType.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\image_file_panel.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\keyboard_ui.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\Shortcut_Button.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\undo.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\widget_panel.h
# End Source File
# Begin Source File

SOURCE=..\..\fluid\WidgetType.h
# End Source File
# End Group
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

SOURCE=..\..\src\FileInput.cxx
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
