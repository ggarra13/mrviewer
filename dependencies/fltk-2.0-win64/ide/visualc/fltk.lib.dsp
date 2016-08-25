# Microsoft Developer Studio Project File - Name="fltk" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=fltk - Win32 Debug Cairo
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fltk.lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fltk.lib.mak" CFG="fltk - Win32 Debug Cairo"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fltk - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fltk - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "fltk - Win32 Release MinSize" (based on "Win32 (x86) Static Library")
!MESSAGE "fltk - Win32 Debug Cairo" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fltk - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\fltk2.lib"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\fltk2d.lib"

!ELSEIF  "$(CFG)" == "fltk - Win32 Release MinSize"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "fltk___Win32_Release_MinSize"
# PROP BASE Intermediate_Dir "fltk___Win32_Release_MinSize"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "fltk___Win32_Release_MinSize"
# PROP Intermediate_Dir "fltk___Win32_Release_MinSize"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Os /Ob2 /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /GR /GX /O1 /Ob2 /I "../.." /I "../../fltk/compat" /I "../visualc" /I "." /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /D "_MSC_DLL" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\fltk2.lib"
# ADD LIB32 /nologo /out:"..\..\lib\fltk2.lib"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug Cairo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "fltk___Win32_Debug_Cairo"
# PROP BASE Intermediate_Dir "fltk___Win32_Debug_Cairo"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "fltk___Win32_Debug_Cairo"
# PROP Intermediate_Dir "fltk___Win32_Debug_Cairo"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "." /I "../.." /I "../../fltk/compat" /I "../visualc" /D USE_CAIRO=1 /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WIN32_LEAN_AND_MEAN" /D "VC_EXTRA_LEAN" /D "WIN32_EXTRA_LEAN" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\..\lib\fltk2d.lib"
# ADD LIB32 /nologo /out:"..\..\lib\fltk2d.lib"

!ENDIF 

# Begin Target

# Name "fltk - Win32 Release"
# Name "fltk - Win32 Debug"
# Name "fltk - Win32 Release MinSize"
# Name "fltk - Win32 Debug Cairo"
# Begin Group "header"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\..\fltk\adjuster.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\aligngroup.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\ask.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\bargroup.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\box.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\browser.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\button.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\checkbutton.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\choice.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\clock.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\color.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\colorchooser.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\ComboBox.h
# End Source File
# Begin Source File

SOURCE=.\config.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\cursor.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\cyclebutton.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\damage.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\dial.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\dirent.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\divider.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\DoubleBufferWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\draw.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\error.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\events.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\file_chooser.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\filebrowser.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\filechooser.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\fileicon.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\fileinput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\filename.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\FillDial.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\FillSlider.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\fl_api.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\FL_VERSION.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\flags.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\floatinput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\font.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\forms.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\gl.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\gl2opengl.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\glut.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\glut0.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\GlWindow.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\group.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\HelpDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\HelpView.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\highlightbutton.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\image.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\input.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\inputbrowser.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\IntInput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\invisiblebox.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\item.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\itemgroup.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\labeltype.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\layout.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\lightbutton.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\LineDial.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\load_plugin.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\mac.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\math.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\menu.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\menubar.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\MenuBuild.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\menuwindow.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\Monitor.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\MultiBrowser.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\multiimage.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\MultiLineInput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\MultiLineOutput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\numericinput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\output.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\packedgroup.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\PixelType.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\popupmenu.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\Preferences.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\progressbar.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\radiobutton.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\RadioItem.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\RadioLightButton.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\rectangle.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\repeatbutton.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\returnbutton.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\rgbimage.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\run.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\scrollbar.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\scrollgroup.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\secretinput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\shapedwindow.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\sharedimage.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\show_colormap.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\slider.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\StatusBarGroup.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\string.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\stringlist.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\style.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\styleset.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\symbol.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\tabgroup.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\textbuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\textdisplay.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\texteditor.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\Threads.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\thumbwheel.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\tiledgroup.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\tiledimage.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\ToggleButton.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\tooltip.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\utf.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\valuator.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\valueinput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\valueoutput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\valueslider.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\visual.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\widget.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\win32.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\window.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\WizardGroup.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\WordwrapInput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\WordwrapOutput.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\x.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\xbmimage.h
# End Source File
# Begin Source File

SOURCE=..\..\fltk\xpmimage.h
# End Source File
# End Group
# Begin Group "source"

# PROP Default_Filter "cxx"
# Begin Source File

SOURCE=..\..\src\add_idle.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\addarc.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\addcurve.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Adjuster.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\AlignGroup.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\args.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\BarGroup.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\bmpImage.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Browser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Button.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\CheckButton.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Choice.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\clip.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Clock.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Color.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\color_chooser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ComboBox.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\compose.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Cursor.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\CycleButton.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\default_glyph.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Dial.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\DiamondBox.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\dnd.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\drawtext.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\EngravedLabel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\error.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\event_key_state.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\file_chooser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\FileBrowser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\FileChooser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\FileChooser2.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\FileIcon.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\FileInput.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_absolute.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_ext.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_isdir.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_list.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_match.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\filename_name.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fillrect.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Fl_Menu_Item.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\FloatInput.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\fltk_theme.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Font.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\gifImage.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Group.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\GSave.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\HelpView.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\HighlightButton.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Image.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Input.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\InputBrowser.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\InvisibleWidget.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Item.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\key_name.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\LightButton.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\list_fonts.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\load_plugin.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\lock.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Menu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Menu_add.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Menu_global.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Menu_popup.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\MenuBar.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\MenuWindow.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\message.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\MultiImage.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\NumericInput.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\numericsort.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Output.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\OvalBox.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\overlay_rect.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\own_colormap.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\PackedGroup.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\path.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\PlasticBox.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\PopupMenu.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Preferences.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ProgressBar.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\RadioButton.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\readimage.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\RepeatButton.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ReturnButton.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\RoundBox.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\RoundedBox.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\run.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\scandir.c
# End Source File
# Begin Source File

SOURCE=..\..\src\Scrollbar.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ScrollGroup.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\scrollrect.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\setcolor.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\setdisplay.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\setvisual.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ShadowBox.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ShapedWindow.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\SharedImage.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ShortcutAssignment.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\show_colormap.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Slider.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\StatusBarGroup.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\string.c
# End Source File
# Begin Source File

SOURCE=..\..\src\StringList.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Style.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\StyleSet.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Symbol.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\TabGroup.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\TabGroup2.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\TextBuffer.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\TextDisplay.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\TextEditor.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ThumbWheel.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\TiledGroup.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\TiledImage.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Tooltip.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\UpBox.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\utf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\Valuator.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ValueInput.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ValueOutput.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\ValueSlider.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\vsnprintf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\Widget.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Widget_draw.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Window.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Window_fullscreen.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Window_hotspot.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\Window_iconize.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\win32\WinMain.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\WizardGroup.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\xbmImage.cxx
# End Source File
# Begin Source File

SOURCE=..\..\src\xpmImage.cxx
# End Source File
# End Group
# Begin Group "documentation"

# PROP Default_Filter ""
# Begin Group "doxygen"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\documentation\Doxyfile

!IF  "$(CFG)" == "fltk - Win32 Release"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug"

!ELSEIF  "$(CFG)" == "fltk - Win32 Release MinSize"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug Cairo"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\documentation\README

!IF  "$(CFG)" == "fltk - Win32 Release"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug"

!ELSEIF  "$(CFG)" == "fltk - Win32 Release MinSize"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug Cairo"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\..\README

!IF  "$(CFG)" == "fltk - Win32 Release"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug"

!ELSEIF  "$(CFG)" == "fltk - Win32 Release MinSize"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug Cairo"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\README.windows

!IF  "$(CFG)" == "fltk - Win32 Release"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug"

!ELSEIF  "$(CFG)" == "fltk - Win32 Release MinSize"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug Cairo"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\README_fltk1_to_fltk2.txt

!IF  "$(CFG)" == "fltk - Win32 Release"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug"

!ELSEIF  "$(CFG)" == "fltk - Win32 Release MinSize"

!ELSEIF  "$(CFG)" == "fltk - Win32 Debug Cairo"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
