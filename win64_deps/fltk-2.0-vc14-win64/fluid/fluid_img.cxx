//
// "$Id"
//
// fluid_img merge all the fluid xpm into one table
//
// Type classes for most of the fltk widgets.  Most of the work
// is done by code in WidgetType.C.  Also a factory instance
// of each of these type classes.
//
// This file also contains the "new" menu, which has a pointer
// to a factory instance for every class (both the ones defined
// here and ones in other files)
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php
//

#include <pixmaps/fluid/lock.xpm>
#include <pixmaps/fluid/flWindow.xpm>
#include <pixmaps/fluid/flButton.xpm>
#include <pixmaps/fluid/flCheckButton.xpm>
#include <pixmaps/fluid/flRoundButton.xpm>
#include <pixmaps/fluid/flBox.xpm>
#include <pixmaps/fluid/flGroup.xpm>
#include <pixmaps/fluid/flFunction.xpm>
#include <pixmaps/fluid/flCode.xpm>
#include <pixmaps/fluid/flCodeBlock.xpm>
#include <pixmaps/fluid/flComment.xpm>
#include <pixmaps/fluid/flDeclaration.xpm>
#include <pixmaps/fluid/flDeclarationBlock.xpm>
#include <pixmaps/fluid/flClass.xpm>
#include <pixmaps/fluid/flNamespace.xpm>
#include <pixmaps/fluid/flTabs.xpm>
#include <pixmaps/fluid/flInput.xpm>
#include <pixmaps/fluid/flChoice.xpm>
#include <pixmaps/fluid/flMenuitem.xpm>
#include <pixmaps/fluid/flMenubar.xpm>
#include <pixmaps/fluid/flSubmenu.xpm>
#include <pixmaps/fluid/flScroll.xpm>
#include <pixmaps/fluid/flTile.xpm>
#include <pixmaps/fluid/flWizard.xpm>
#include <pixmaps/fluid/flPack.xpm>
#include <pixmaps/fluid/flReturnButton.xpm>
#include <pixmaps/fluid/flLightButton.xpm>
#include <pixmaps/fluid/flRepeatButton.xpm>
#include <pixmaps/fluid/flMenuButton.xpm>
#include <pixmaps/fluid/flOutput.xpm>
#include <pixmaps/fluid/flTextDisplay.xpm>
#include <pixmaps/fluid/flTextEdit.xpm>
#include <pixmaps/fluid/flFileInput.xpm>
#include <pixmaps/fluid/flBrowser.xpm>
#include <pixmaps/fluid/flCheckBrowser.xpm>
#include <pixmaps/fluid/flFileBrowser.xpm>
#include <pixmaps/fluid/flClock.xpm>
#include <pixmaps/fluid/flHelp.xpm>
#include <pixmaps/fluid/flProgress.xpm>
#include <pixmaps/fluid/flSlider.xpm>
#include <pixmaps/fluid/flScrollBar.xpm>
#include <pixmaps/fluid/flValueSlider.xpm>
#include <pixmaps/fluid/flAdjuster.xpm>
#include <pixmaps/fluid/flCounter.xpm>
#include <pixmaps/fluid/flDial.xpm>
#include <pixmaps/fluid/flRoller.xpm>
#include <pixmaps/fluid/flValueInput.xpm>
#include <pixmaps/fluid/flValueOutput.xpm>
#include <pixmaps/fluid/flSpinner.xpm>
#include <pixmaps/fluid/flWidgetClass.xpm>
//#include <pixmaps/file_small.xpm>
//#include <pixmaps/folder_save.xpm>
#include <pixmaps/file_small2.xpm>
#include <pixmaps/folder_small.xpm>
#include <pixmaps/book.xpm>
#include <pixmaps/recycle.xpm>

#include "fluid_img.h"
#include <fltk/xpmImage.h>

using namespace fltk;

xpmImage	open_pixmap(folder_small);
xpmImage	new_pixmap(file_small2);
xpmImage	write_pixmap(book);
xpmImage	save_pixmap(book);
xpmImage	saveas_pixmap(book);
xpmImage	merge_pixmap(recycle_xpm);
xpmImage	exit_pixmap(recycle_xpm);

static xpmImage	lock_pixmap(lock_xpm);
static xpmImage	window_pixmap(flWindow_xpm);
static xpmImage	button_pixmap(flButton_xpm);
static xpmImage	checkbutton_pixmap(flCheckButton_xpm);
static xpmImage	roundbutton_pixmap(flRoundButton_xpm);
static xpmImage	box_pixmap(flBox_xpm);
static xpmImage	group_pixmap(flGroup_xpm);
static xpmImage	function_pixmap(flFunction_xpm);
static xpmImage	code_pixmap(flCode_xpm);
static xpmImage	codeblock_pixmap(flCodeBlock_xpm);
static xpmImage	comment_pixmap(flComment_xpm);
static xpmImage	declaration_pixmap(flDeclaration_xpm);
static xpmImage	declarationblock_pixmap(flDeclarationBlock_xpm);
static xpmImage	class_pixmap(flClass_xpm);
static xpmImage	namespace_pixmap(flNamespace_xpm);
static xpmImage	tabs_pixmap(flTabs_xpm);
static xpmImage	input_pixmap(flInput_xpm);
static xpmImage	choice_pixmap(flChoice_xpm);
static xpmImage	menuitem_pixmap(flMenuitem_xpm);
static xpmImage	menubar_pixmap(flMenubar_xpm);
static xpmImage	submenu_pixmap(flSubmenu_xpm);
static xpmImage	scroll_pixmap(flScroll_xpm);
static xpmImage	tile_pixmap(flTile_xpm);
static xpmImage	wizard_pixmap(flWizard_xpm);
static xpmImage	pack_pixmap(flPack_xpm);
static xpmImage	returnbutton_pixmap(flReturnButton_xpm);
static xpmImage	lightbutton_pixmap(flLightButton_xpm);
static xpmImage	repeatbutton_pixmap(flRepeatButton_xpm);
static xpmImage	menubutton_pixmap(flMenuButton_xpm);
static xpmImage	output_pixmap(flOutput_xpm);
static xpmImage	textdisplay_pixmap(flTextDisplay_xpm);
static xpmImage	textedit_pixmap(flTextEdit_xpm);
static xpmImage	fileinput_pixmap(flFileInput_xpm);
static xpmImage	browser_pixmap(flBrowser_xpm);
static xpmImage	checkbrowser_pixmap(flCheckBrowser_xpm);
static xpmImage	filebrowser_pixmap(flFileBrowser_xpm);
static xpmImage	clock_pixmap(flClock_xpm);
static xpmImage	help_pixmap(flHelp_xpm);
static xpmImage	progress_pixmap(flProgress_xpm);
static xpmImage	slider_pixmap(flSlider_xpm);
static xpmImage	scrollbar_pixmap(flScrollBar_xpm);
static xpmImage	valueslider_pixmap(flValueSlider_xpm);
static xpmImage	adjuster_pixmap(flAdjuster_xpm);
static xpmImage	counter_pixmap(flCounter_xpm);
static xpmImage	dial_pixmap(flDial_xpm);
static xpmImage	roller_pixmap(flRoller_xpm);
static xpmImage	valueinput_pixmap(flValueInput_xpm);
static xpmImage	valueoutput_pixmap(flValueOutput_xpm);
static xpmImage	spinner_pixmap(flSpinner_xpm);
static xpmImage	widgetclass_pixmap(flWidgetClass_xpm);

xpmImage * fluid_pixmap[] = { 
 0, &window_pixmap, &button_pixmap, &checkbutton_pixmap, &roundbutton_pixmap, /* 0..4 */
 &box_pixmap, &group_pixmap, &function_pixmap, &code_pixmap, &codeblock_pixmap, &declaration_pixmap, /* 5..10 */ 
 &declarationblock_pixmap, &class_pixmap, &tabs_pixmap, &input_pixmap, &choice_pixmap,               /* 11..15 */
 &menuitem_pixmap, &menubar_pixmap, &submenu_pixmap, &scroll_pixmap, &tile_pixmap, &wizard_pixmap,   /* 16..21 */
 &pack_pixmap, &returnbutton_pixmap, &lightbutton_pixmap, &repeatbutton_pixmap, &menubutton_pixmap,  /* 22..26 */
 &output_pixmap, &textdisplay_pixmap, &textedit_pixmap, &fileinput_pixmap, &browser_pixmap,          /* 27..32 */
 &checkbrowser_pixmap, &filebrowser_pixmap, &clock_pixmap, &help_pixmap, &progress_pixmap,	     /* 33..36 */
 &slider_pixmap, &scrollbar_pixmap, &valueslider_pixmap, &adjuster_pixmap, &counter_pixmap,          /* 37..41 */
 &dial_pixmap, &roller_pixmap, &valueinput_pixmap, &valueoutput_pixmap, &comment_pixmap,             /* 42..46 */
 &spinner_pixmap, &widgetclass_pixmap,&namespace_pixmap /* 47..49 */ 
};

// End of "$Id"
