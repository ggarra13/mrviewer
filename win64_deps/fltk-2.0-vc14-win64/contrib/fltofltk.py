#!/bin/python
#
# "$Id: $"
# 
# fltofltk.py -- convert FLTK 1.x code to FLTK 2.x
#
# Copyright(C) 2005 Chris Warren-Smith. Gawler, South Australia
# cwarrens@twpo.com.au
#
# This program is distributed under the terms of the GPL v2.0 or later
# Download the GNU Public License (GPL) from www.gnu.org
#

import sys
import os
import glob
import re
import shutil

backupDir = "v1src.bak"
version = "$Revision: 0.2 $";
versionStr = version.replace("Revision:", "").replace("$", "")
codeComment = "// Converted to FLTK~2 using fltofltk.py version"+versionStr+ "\n"

FL_INCLUDE="\
\n//TODO: delete unused header files\
\n#include <fltk/events.h>\
\n#include <fltk/run.h>\
\n#include <fltk/events.h>\
\n#include <fltk/Color.h>\
\n#include <fltk/Font.h>\
\n#include <fltk/Box.h>\
\n#include <fltk/error.h>\
\n#include <fltk/visual.h>\
\n#include <fltk/damage.h>\
\n#include <fltk/filename.h>\
\n#include <fltk/Cursor.h>\
"

SCROLL_INCLUDE="\
#include <fltk/ScrollGroup.h>\
\n#include <fltk/Scrollbar.h>\
"

def refactor(s, oldfunc, newfunc, numargs):
   # patch oldfunc function calls in s with newfunc calls
   # eg foo_func(x) -> setFoo(x);
   nextIndex = 0
   while 1:
      i = s.find(oldfunc, nextIndex)
      if (i == -1):
         break
      argBegin = s.find("(", i)
      if (argBegin == -1):
         break;
      argEnd = s.find(")", argBegin)
      if (argEnd == -1):
         break;

      # check to ensure oldfunc and '(' are on the same line
      lineBreak = s.find("\n", i)
      if (lineBreak < argBegin):
         nextIndex = lineBreak+1
         continue

      # create a list of program arguments
      args = s[argBegin+1:argEnd]
      argList = []
      if (len(args) > 0):
         nextWord = ""           
         for c in args:
            if (c == ','):
               argList.append(nextWord)
               nextWord = ""
            else:
               nextWord = nextWord+c
         argList.append(nextWord)

      # check this is the desired overload/signature           
      if (len(argList) != numargs):
         nextIndex = argEnd
         continue

      # replace the argument place-holders in newfunc with the
      # existing code arguments from the v1 code
      for argn in range(1, numargs+1):
         newfuncCode = re.sub(str(argn), argList[argn-1], newfunc)

      # update the next code string
      s = s[:i]+newfuncCode+s[argEnd+1:]
      nextIndex = i+len(newfuncCode)
   return s

def noopCode(s, oldfunc):
   # remove the entire call to the given function
   nextIndex = 0
   while 1:
      i = s.find(oldfunc, nextIndex)
      if (i == -1):
         break
      iend = s.find("(", i)
      if (iend == -1):
         break;
      iend = s.find(")", iend)
      if (iend == -1):
         break;
      iend = s.find(";", iend)
      if (iend == -1):
         break;
      # update the next code string
      s = s[:i]+s[iend+1:]
      nextIndex = i
   return s

def processHashDefines(s):
   # process #define convervions from FL mapping headers
   s = s.replace("FL_UP_FRAME", "UP_BOX")
   s = s.replace("FL_DOWN_FRAME", "DOWN_BOX")
   s = s.replace("FL_THIN_UP_FRAME", "THIN_UP_BOX")
   s = s.replace("FL_THIN_DOWN_FRAME", "THIN_DOWN_BOX")
   s = s.replace("FL_ENGRAVED_FRAME", "ENGRAVED_BOX")
   s = s.replace("FL_EMBOSSED_FRAME", "EMBOSSED_BOX")
   s = s.replace("FL_ROUNDED_FRAME", "ROUNDED_BOX")
   s = s.replace("FL_OVAL_FRAME", "OVAL_BOX")
   s = s.replace("FL_PLASTIC_UP_FRAME", "PLASTIC_UP_BOX")
   s = s.replace("FL_PLASTIC_DOWN_FRAME", "PLASTIC_DOWN_BOX")
   s = s.replace("FL_FRAME", "FL_ENGRAVED_FRAME")
   s = s.replace("FL_FRAME_BOX", "FL_ENGRAVED_BOX")
   s = s.replace("FL_CIRCLE_BOX", "FL_ROUND_DOWN_BOX")
   s = s.replace("FL_DIAMOND_BOX", "FL_DIAMOND_DOWN_BOX")
   s = s.replace("FL_SYMBOL_LABEL", "FL_NORMAL_LABEL")
   s = s.replace("Fl_Align", "Flags")
   s = s.replace("FL_SYMBOL", "SYMBOL_FONT")
   s = s.replace("FL_SCREEN", "SCREEN_FONT")
   s = s.replace("FL_SCREEN_BOLD", "SCREEN_BOLD_FONT")
   s = s.replace("FL_BOLD", "HELVETICA_BOLD")
   s = s.replace("FL_ITALIC", "HELVETICA_ITALIC")
   s = s.replace("FL_NORMAL_SIZE", "(Widget::default_style->labelsize())")
   s = s.replace("FL_FOREGROUND_COLOR", "(Widget::default_style->labelcolor_)")
   s = s.replace("FL_BACKGROUND2_COLOR", "(Widget::default_style->color_)")
   s = s.replace("FL_SELECTION_COLOR", "(Widget::default_style->selection_color_)")
   s = s.replace("fl_color_average", "lerp")
   s = s.replace("fl_lighter", "lerp(WHITE,1,.67f)", 1)
   s = s.replace("fl_darker", "lerp(BLACK,1,.67f)", 1)
   s = s.replace("fl_rgb_color", "color")
   s = refactor(s, "fl_gray_ramp", "(GRAY00+(1))", 1)
   s = s.replace("Fl_Cursor", "Cursor*")
   s = s.replace("FL_CURSOR_N", "CURSOR_NS")
   s = s.replace("FL_CURSOR_NE", "CURSOR_NESW")
   s = s.replace("FL_CURSOR_E", "CURSOR_WE")
   s = s.replace("FL_CURSOR_SE", "CURSOR_NWSE")
   s = s.replace("FL_CURSOR_S", "CURSOR_NS")
   s = s.replace("FL_CURSOR_SW", "CURSOR_NESW")
   s = s.replace("FL_CURSOR_W", "CURSOR_WE")
   s = s.replace("FL_CURSOR_NW", "CURSOR_NWSE")
   s = s.replace("FL_FILLED_CHART",  "FL_FILL_CHART")
   s = s.replace("fl_mult_matrix", "concat")
   s = s.replace("fl_begin_points", "newpath")
   s = s.replace("fl_begin_line", "newpath")
   s = s.replace("fl_begin_loop", "newpath")
   s = s.replace("fl_begin_polygon", "newpath")
   s = s.replace("fl_vertex", "addvertex")
   s = s.replace("fl_curve", "addcurve")
   s = s.replace("fl_circle", "addcircle")
   s = s.replace("fl_end_points", "drawpoints")
   s = s.replace("fl_end_line", "strokepath")
   s = s.replace("fl_end_polygon", "fillpath")
   s = s.replace("fl_begin_complex_polygon", "newpath")
   s = s.replace("fl_gap", "closepath")
   s = s.replace("fl_end_complex_polygon", "fillpath")
   s = s.replace("fl_draw_pixmap", "draw_xpm")
   s = s.replace("fl_measure_pixmap", "measure_xpm")
   s = s.replace("fl_scroll", "scrollrect")
   s = s.replace("fl_shortcut_label", "key_name")
   s = s.replace("FL_WHEN_NEEDED", "FL_AUTO")
   s = s.replace("FL_LCOL", "BLACK")
   s = s.replace("FL_COL1", "GRAY75")
   s = s.replace("FL_MCOL", "GRAY85")
   s = s.replace("FL_LEFT_BCOL", "GRAY99")
   s = s.replace("FL_TOP_BCOL", "GRAY90")
   s = s.replace("FL_BOTTOM_BCOL", "GRAY35")
   s = s.replace("FL_RIGHT_BCOL", "GRAY20")
   s = s.replace("FL_INACTIVE", "GRAY66")
   s = s.replace("FL_INACTIVE_COL", "GRAY66")
   s = s.replace("FL_FREE_COL1", "Color(16)")
   s = s.replace("FL_FREE_COL2", "Color(17)")
   s = s.replace("FL_FREE_COL3", "Color(18)")
   s = s.replace("FL_FREE_COL4", "Color(19)")
   s = s.replace("FL_FREE_COL5", "Color(20)")
   s = s.replace("FL_FREE_COL6", "Color(21)")
   s = s.replace("FL_FREE_COL7", "Color(22)")
   s = s.replace("FL_FREE_COL8", "Color(23)")
   s = s.replace("FL_FREE_COL9", "Color(24)")
   s = s.replace("FL_FREE_COL10", "Color(25)")
   s = s.replace("FL_FREE_COL11", "Color(26)")
   s = s.replace("FL_FREE_COL12", "Color(27)")
   s = s.replace("FL_FREE_COL13", "Color(28)")
   s = s.replace("FL_FREE_COL14", "Color(29)")
   s = s.replace("FL_FREE_COL15", "Color(30)")
   s = s.replace("FL_FREE_COL16", "Color(31)")
   s = s.replace("FL_TOMATO", "Color(131)")
   s = s.replace("FL_INDIANRED", "Color(164)")
   s = s.replace("FL_SLATEBLUE", "Color(195)")
   s = s.replace("FL_DARKGOLD", "Color(84)")
   s = s.replace("FL_PALEGREEN", "Color(157)")
   s = s.replace("FL_ORCHID", "Color(203)")
   s = s.replace("FL_DARKCYAN", "Color(189)")
   s = s.replace("FL_DARKTOMATO", "Color(113)")
   s = s.replace("FL_WHEAT", "Color(174)")
   s = s.replace("FL_ALIGN_BESIDE", "ALIGN_INSIDE")
   s = s.replace("FL_NO_FRAME", "NO_BOX")
   s = s.replace("FL_ROUNDED3D_UPBOX", "ROUND_UP_BOX")
   s = s.replace("FL_ROUNDED3D_DOWNBOX", "ROUND_DOWN_BOX")
   s = s.replace("FL_OVAL3D_UPBOX", "ROUND_UP_BOX")
   s = s.replace("FL_OVAL3D_DOWNBOX", "ROUND_DOWN_BOX")
   s = s.replace("FL_DEFAULT_SIZE", "FL_SMALL_SIZE")
   s = s.replace("FL_TINY_FONT", "FL_TINY_SIZE")
   s = s.replace("FL_SMALL_FONT", "FL_SMALL_SIZE")
   s = s.replace("FL_NORMAL_FONT", "FL_NORMAL_SIZE")
   s = s.replace("FL_MEDIUM_FONT", "FL_MEDIUM_SIZE")
   s = s.replace("FL_LARGE_FONT", "FL_LARGE_SIZE")
   s = s.replace("FL_HUGE_FONT", "FL_HUGE_SIZE")
   s = s.replace("FL_NORMAL_FONT1", "FL_SMALL_FONT")
   s = s.replace("FL_NORMAL_FONT2", "FL_NORMAL_FONT")
   s = s.replace("FL_DEFAULT_FONT", "FL_SMALL_FONT")
   s = s.replace("FL_RETURN_END_CHANGED", "FL_WHEN_RELEASE")
   s = s.replace("FL_RETURN_CHANGED", "FL_WHEN_CHANGED")
   s = s.replace("FL_RETURN_END", "FL_WHEN_RELEASE_ALWAYS")
   s = s.replace("FL_RETURN_ALWAYS", "(FL_WHEN_CHANGED|FL_WHEN_NOT_CHANGED)")
   s = s.replace("FL_CMD_OPT", "void")
   s = s.replace("fl_set_object_focus", "fl_set_focus_object")
   s = s.replace("fl_set_object_lcolor", "fl_set_object_lcol")
   s = s.replace("fl_set_object_align", "fl_set_object_lalign")
   s = s.replace("fl_set_call_back", "fl_set_object_callback")
   s = s.replace("resizebox", "_ddfdesign_kludge()")
   s = s.replace("fl_set_initial_placement", "fl_set_form_geometry")
   s = s.replace("FL_PLACE_FREE_CENTER", "(FL_PLACE_CENTER|FL_FREE_SIZE)")
   s = s.replace("FL_PLACE_CENTERFREE", "(FL_PLACE_CENTER|FL_FREE_SIZE)")
   s = s.replace("fl_prepare_form_window", "fl_show_form")
   s = s.replace("fl_set_form_call_back", "fl_set_form_callback")
   s = s.replace("fl_mousebutton", "fl_mouse_button")
   s = s.replace("fl_set_clipping", "fl_clip")
   s = s.replace("fl_unset_clipping", "fl_pop_clip")
   s = s.replace("FL_NORMAL_BITMAP", "FL_NO_BOX")
   s = s.replace("FL_NORMAL_PIXMAP", "FL_NO_BOX")
   s = s.replace("FL_PUSH_BUTTON", "FL_TOGGLE_BUTTON")
   s = s.replace("fl_set_button_shortcut", "fl_set_object_shortcut")
   s = s.replace("fl_set_chart_lcol", "fl_set_chart_lcolor")
   s = s.replace("FL_INVISIBLE_CURSOR", "FL_CURSOR_NONE")
   s = s.replace("FL_DEFAULT_CURSOR", "FL_CURSOR_DEFAULT")
   s = s.replace("FL_DIAL_COL1", "FL_GRAY")
   s = s.replace("fl_show_messages", "fl_message")
   s = s.replace("FL_HOR_BROWSER_SLIDER", "FL_HOR_SLIDER")
   s = s.replace("FL_VERT_BROWSER_SLIDER", "FL_VERT_SLIDER")
   s = s.replace("FL_SLIDER_COL1", "FL_GRAY")
   s = s.replace("FL_NORMAL_TEXT", "FL_NO_BOX")
   s = s.replace("gl_start", "glstart")
   s = s.replace("gl_finish", "glfinish")
   s = s.replace("gl_color", "glsetcolor")
   s = s.replace("gl_rect", "glstrokerect")
   s = s.replace("gl_rectf", "glfillrect")
   s = s.replace("gl_font", "glsetfont")
   s = s.replace("gl_descent", "glgetdescent")
   s = s.replace("gl_width", "glgetwidth")
   s = s.replace("gl_draw", "gldrawtext")
   s = s.replace("gl_draw_image", "gldrawimage")
   s = s.replace("CHECKED", "FL_MENU_CHECK")
   s = s.replace("UNCHECKED", "FL_MENU_BOX")
   s = s.replace("DISABLED", "FL_MENU_INACTIVE")
   s = s.replace("Fl_X", "CreatedWindow")
   s = s.replace("fl_display", "xdisplay")
   s = s.replace("fl_screen", "xscreen")
   s = s.replace("fl_xevent", "(&xevent)")
   s = s.replace("fl_colormap", "xcolormap")
   s = s.replace("fl_open_display", "open_display")
   s = s.replace("fl_visual", "xvisual")
   s = s.replace("fl_window", "xwindow")
   return s

def processInlines(s):
   # process simple single-line inline convervions from FL mapping headers
   s = s.replace("Fl::x()", "Monitor::all().x()")
   s = s.replace("Fl::y()", "Monitor::all().y()")
   s = s.replace("Fl::w()", "Monitor::all().w()")
   s = s.replace("Fl::h()", "Monitor::all().x()")
   s = s.replace("get_color", "/*FIXME:get_color_index|split_color */")
   s = refactor(s, "get_font_sizes", "sizes", 0)
   s = refactor(s, "box_dx", "1->dx()", 1)
   s = refactor(s, "box_dy", "1->dy()", 1)
   s = refactor(s, "box_dw", "1->dw()", 1)
   s = refactor(s, "box_dh", "1->dh()", 1)
   s = refactor(s, "event_key", "event_key_state(1)", 1)

   # process fl_draw.H (TODO complete me)
   s = s.replace("fl_color", "setcolor /*FIXME:getcolor*/")
   s = s.replace("fl_push_clip", "push_clip(Rectangle")
   s = s.replace("fl_not_clipped", "not_clipped(Rectangle")
   s = s.replace("fl_point", "addvertex /*FIXME:drawpoints();*/")
   s = s.replace("fl_rect", "strokerect(Rectangle")
   s = s.replace("fl_rectf", "fillrect(Rectangle")
   s = s.replace("fl_line", "drawline")
   s = s.replace("fl_xyline", "drawline /*FIXME:addvertex|strokepath*/")
   s = s.replace("fl_yxline", "drawline /*FIXME:addvertex|strokepath*/")
   s = s.replace("fl_arc", "addchord(Rectangle /*FIXME:strokepath*/")
   s = s.replace("fl_pie", "addpie(Rectangle /*FIXME:fillpath*/")
   s = s.replace("fl_chord", "addchord(Rectangle /*FIXME:fillpath*/")
   s = s.replace("fl_end_loop()", "fltk::closepath();fltk::strokepath()")
   s = s.replace("fl_font", "setfont /*FIXME:getfont*/")
   s = s.replace("fl_size", "getsize")
   s = s.replace("fl_height()", "getascent()+getdescent()")
   s = s.replace("fl_descent", "getdescent")
   s = s.replace("fl_width", "getwidth")
   s = s.replace("fl_draw", "drawtext")
   s = s.replace("fl_draw_image", "drawimage")
   s = s.replace("fl_read_image", "readimage")
   s = refactor(s, "fl_rectf", "setcolor(fltk::color(5,6,7));fillrect(Rectangle(1,2,3,4))", 7)
   s = s.replace("fl_set_idle_callback", "set_idle")
   s = s.replace("fl_do_only_forms", "fl_do_forms")
   s = s.replace("fl_check_only_forms", "fl_check_forms")
   s = s.replace("fl_set_focus_object", "focus /*TODO:fixme*/")
   s = refactor(s, "fl_set_object_boxtype", "1->box(2)", 2)
   s = refactor(s, "fl_set_object_lsize", "1->label_size(2)", 2)
   s = refactor(s, "fl_set_object_lcol", "1->label_color((Fl_Color)2)", 2)
   s = refactor(s, "fl_set_object_lalign", "1->clear_flag(FL_ALIGN_MASK);1->set_flag(2)", 2)
   s = refactor(s, "fl_set_object_color", "1->color((Fl_Color)2); 1->selection_color((Fl_Color)3)", 3);
   s = refactor(s, "fl_set_object_label", "1->label(2); 1->redraw()", 2)
   s = refactor(s, "fl_set_object_position", "1->position(2,3)", 3)
   s = refactor(s, "fl_set_object_size", "1->size(2,3)", 3)
   s = refactor(s, "fl_set_object_geometry", "1->resize(2,3,4,5)", 5)
   s = refactor(s, "fl_set_object_callback", "1->callback(2,3)", 3)
   s = refactor(s, "fl_call_object_callback", "1->do_callback()", 1)
   s = refactor(s, "fl_trigger_object", "1->do_callback()", 1)
   s = refactor(s, "fl_redraw_object", "1o->redraw()", 1)
   s = refactor(s, "fl_show_object", "1->show()", 1)
   s = refactor(s, "fl_hide_object", "1->hide()", 1)
   s = refactor(s, "fl_free_object", "delete 1;", 1)
   s = refactor(s, "fl_delete_object", "(Fl_Group*)(1->parent()))->remove(1))", 1)
   s = refactor(s, "fl_activate_object", "1->activate()", 1)
   s = refactor(s, "fl_deactivate_object", "1->deactivate()", 1)
   s = refactor(s, "fl_add_object", "1->add(2)", 2)
   s = refactor(s, "FL_ObjWin", "1->window()", 1)
   s = s.replace("fl_get_border_width", "3")
   s = noopCode(s, "fl_set_border_width")
   s = noopCode(s, "fl_set_object_dblbuffer")
   s = noopCode(s, "fl_set_form_dblbuffer")
   s = refactor(s, "fl_free_form", "delete 1" ,1)
   s = refactor(s, "fl_redraw_form", "1->redraw()", 1)
   s = refactor(s, "fl_addto_form", "1->begin()", 1)
   s = s.replace("fl_bgn_group()", "new Fl_Group(0,0,0,0,0)")
   s = refactor(s, "fl_addto_group", "((Fl_Group*)1)->begin()", 1)
   s = refactor(s, "fl_set_form_position", "1->position(2,3)", 3)
   s = refactor(s, "fl_set_form_size", "1->size(2,3)", 3)
   s = noopCode(s, "fl_adjust_form_size")
   s = refactor(s, "fl_set_form_hotspot", "1->hotspot(2,3)", 3)
   s = refactor(s, "fl_set_form_hotobject", "1->hotspot(2)", 2)
   s = refactor(s, "fl_flip_yorigin", "fl_flip = 1", 0)
   s = noopCode(s, "fl_show_form_window")
   s = refactor(s, "fl_raise_form", "1->show()", 1)
   s = refactor(s, "fl_hide_form", "1->hide()", 1)
   s = refactor(s, "fl_pop_form", "1->show()", 1)
   s = noopCode(s, "fl_activate_all_forms")
   s = refactor(s, "fl_deactivate_all_forms", "fl_modal_next = 1", 0)
   s = refactor(s, "fl_deactivate_form", "1->deactivate()", 1)
   s = refactor(s, "fl_activate_form", "1->activate()", 1)
   s = refactor(s, "fl_set_form_title", "1->label(2)", 2)
   s = refactor(s, "fl_title_form", "1->label(2)", 2)
   s = refactor(s, "fl_set_form_callback", "1->callback(2)", 2)
   s = noopCode(s, "fl_init");
   s = refactor(s, "fl_form_is_visible", "1->visible()", 1)
   s = s.replace("fl_mouse_button", "event_button")
   s = noopCode(s, "fl_add_new")
   s = refactor(s, "fl_add_new", "2->type(1); return 2", 2)
   s = refactor(s, "fl_set_pixmap_align", "1->clear_flag(FL_ALIGN_MASK); 1->set_flag(2)", 2)
   s = refactor(s, "fl_get_button", "((Fl_Button*)1)->value()", 1)
   s = refactor(s, "fl_set_button", "((Fl_Button*)1)->value(2)", 2)
   s = refactor(s, "fl_get_button_numb", "event_button()", 0)
   s = refactor(s, "fl_add_bitmapbutton", "fl_add_button(1,2,3,4,5)", 5)
   s = refactor(s, "fl_add_pixmapbutton", "fl_add_button(1,2,3,4,5,6)", 6)
   s = refactor(s, "fl_set_cursor", "1->cursor(2)", 2)
   s = refactor(s, "fl_show_question", "fl_ask(1)", 1)
   s = refactor(s, "fl_show_question", "fl_ask(1)", 2)   
   s = refactor(s, "fl_show_input", "fl_input(1,2)", 1)
   s = refactor(s, "fl_show_input", "fl_input(1,2)", 2)   
   s = refactor(s, "do_matching", "filename_match(1,2)", 2)
   s = refactor(s, "fl_get_input", "((Fl_Input*)1)->value()", 1)
   s = refactor(s, "fl_set_timer", "((Fl_Timer*)1)->value(2)", 2)
   s = refactor(s, "fl_get_timer", "((Fl_Timer*)1)->value()", 1)
   s = refactor(s, "fl_suspend_timer", "((Fl_Timer*)1)->suspended(_ONE_)", 1)
   s = refactor(s, "fl_resume_timer", "((Fl_Timer*)1)->suspended(1)", 1)
   s = refactor(s, "fl_set_timer_countup", "((Fl_Timer*)1)->direction(2))", 2)
   s = s.replace("fl_double_click()", "event_clicks()")
   s = s.replace("fl_draw", "flush()")
   s = s.replace("gl_height()", "fltk::glgetascent()+fltk::glgetdescent()")
   return s

def processKeyboard(s):
   # update keyboard constants
   s = s.replace("FL_BackSpace", "BackSpaceKey")
   s = s.replace("FL_Tab", "TabKey")
   s = s.replace("FL_Enter", "ReturnKey")
   s = s.replace("FL_Pause", "PauseKey")
   s = s.replace("FL_Scroll_Lock", "ScrollLockKey")
   s = s.replace("FL_Escape", "EscapeKey")
   s = s.replace("FL_Home", "HomeKey")
   s = s.replace("FL_Left", "LeftKey")
   s = s.replace("FL_Up", "UpKey")
   s = s.replace("FL_Right", "RightKey")
   s = s.replace("FL_Down", "DownKey")
   s = s.replace("FL_Page_Up", "PageUpKey")
   s = s.replace("FL_Page_Down", "PageDownKey")
   s = s.replace("FL_End", "EndKey")
   s = s.replace("FL_Print", "PrintKey")
   s = s.replace("FL_Insert", "InsertKey")
   s = s.replace("FL_Menu", "MenuKey")
   s = s.replace("FL_Help", "HelpKey")
   s = s.replace("FL_Num_Lock", "NumLockKey")
   s = s.replace("FL_KP_Enter", "KeypadEnter")
   s = s.replace("FL_KP_Last", "KeypadLast")
   s = s.replace("FL_KP", "Keypad")
   s = s.replace("FL_Shift_L", "LeftShiftKey")
   s = s.replace("FL_Shift_R", "RightShiftKey")
   s = s.replace("FL_Control_L", "LeftCtrlKey")
   s = s.replace("FL_Control_R", "RightCtrlKey")
   s = s.replace("FL_Caps_Lock", "CapsLockKey")
   s = s.replace("FL_Meta_L", "LeftMetaKey")
   s = s.replace("FL_Meta_R", "RightMetaKey")
   s = s.replace("FL_Alt_L", "LeftAltKey")
   s = s.replace("FL_Alt_R", "RightAltKey")
   s = s.replace("FL_Delete", "DeleteKey")
   return s

def processEnumerations(s):
   # process enums from Enumerations.H
   s = s.replace("FL_KEYDOWN", "KEY")
   s = s.replace("FL_KEYBOARD", "KEY")
   # replace colors
   s = s.replace("FL_GRAY0", "GRAY00")
   s = s.replace("FL_DARK3", "GRAY33")
   s = s.replace("FL_DARK2", "GRAY60")
   s = s.replace("FL_DARK1", "GRAY66")
   s = s.replace("FL_GRAY", "GRAY75")
   s = s.replace("FL_BACKGROUND_COLOR", "GRAY75")
   s = s.replace("FL_LIGHT1", "GRAY85")
   s = s.replace("FL_LIGHT2", "GRAY90")
   s = s.replace("FL_LIGHT3", "GRAY99")
   s = s.replace("FL_DARK_RED", "fltk::color(128,0,0)")
   s = s.replace("FL_DARK_GREEN", "fltk::color(0,128,0)")
   s = s.replace("FL_DARK_YELLOW", "fltk::color(128,128,0)")
   s = s.replace("FL_DARK_BLUE", "fltk::color(0,0,128)")
   s = s.replace("FL_DARK_MAGENTA", "fltk::color(128,0,128)")
   s = s.replace("FL_DARK_CYAN", "fltk::color(0,128,128)")
   s = s.replace("FL_GRAY_RAMP", "GRAY00")
   s = s.replace("FL_COLOR_CUBE", "BLACK")
   # replace Flags.h
   s = s.replace("FL_ALIGN_TEXT_OVER_IMAGE", "ALIGN_BOTTOM|ALIGN_INSIDE")
   s = s.replace("FL_ALIGN_TOP_LEFT", "ALIGN_TOP|ALIGN_LEFT")
   s = s.replace("FL_ALIGN_TOP_RIGHT", "ALIGN_TOP|ALIGN_RIGHT")
   s = s.replace("FL_ALIGN_BOTTOM_LEFT", "ALIGN_BOTTOM|ALIGN_LEFT")
   s = s.replace("FL_ALIGN_BOTTOM_RIGHT", "ALIGN_BOTTOM|ALIGN_RIGHT")
   s = s.replace("FL_ALIGN_LEFT_TOP", "ALIGN_LEFT|ALIGN_TOP")
   s = s.replace("FL_ALIGN_RIGHT_TOP", "ALIGN_RIGHT|ALIGN_TOP")
   s = s.replace("FL_ALIGN_LEFT_BOTTOM", "ALIGN_LEFT|ALIGN_BOTTOM")
   s = s.replace("FL_ALIGN_RIGHT_BOTTOM", "ALIGN_RIGHT|ALIGN_BOTTOM")
   # replace Browser.h
   s = s.replace("FL_NORMAL_BROWSER", "Browser::NORMAL")
   s = s.replace("FL_SELECT_BROWSER", "Browser::NORMAL")
   s = s.replace("FL_HOLD_BROWSER", "Browser::NORMAL")
   s = s.replace("FL_MULTI_BROWSER", "Browser::MULTI")
   s = s.replace("FL_MULTI_BROWSER", "Browser::MULTI")  
   s = s.replace("FL_NORMAL_BUTTON", "Button::NORMAL")
   s = s.replace("FL_TOGGLE_BUTTON", "Button::TOGGLE")
   s = s.replace("FL_RADIO_BUTTON", "Button::RADIO")
   s = s.replace("FL_HIDDEN_BUTTON", "Button::HIDDEN")
   s = s.replace("FL_SQUARE_CLOCK", "Clock::SQUARE")
   s = s.replace("FL_ROUND_CLOCK", "Clock::ROUND")
   s = s.replace("FL_ANALOG_CLOCK", "Clock::SQUARE")
   s = s.replace("FL_DIGITAL_CLOCK", "Clock::DIGITAL")
   s = s.replace("FL_NORMAL_DIAL", "Dial::NORMAL")
   s = s.replace("FL_LINE_DIAL", "Dial::LINE")
   s = s.replace("FL_FILL_DIAL", "Dial::FILL")
   s = s.replace("FL_NORMAL_INPUT", "Input::NORMAL")
   s = s.replace("FL_MULTILINE_INPUT", "Input::MULTILINE")
   s = s.replace("FL_SECRET_INPUT", "Input::SECRET")
   s = s.replace("FL_INPUT_WRAP", "Input::WORDWRAP")
   s = s.replace("FL_MULTILINE_INPUT_WRAP", "Input::WORDWRAP")
   s = s.replace("FL_NORMAL_OUTPUT", "Input::NORMAL")
   s = s.replace("FL_MULTILINE_OUTPUT", "Input::MULTILINE")
   s = s.replace("FL_MULTILINE_OUTPUT_WRAP", "Input::WORDWRAP")
   s = s.replace("FL_VERT_SLIDER", "Slider::LINEAR")
   s = s.replace("FL_HOR_SLIDER", "Slider::HORIZONTAL")
   s = s.replace("FL_VERT_FILL_SLIDER", "Slider::FILL")
   s = s.replace("FL_HOR_FILL_SLIDER", "Slider::HORIZONTAL+fltk::Slider::FILL")
   s = s.replace("FL_VERT_NICE_SLIDER", "Slider::LINEAR")
   s = s.replace("FL_HOR_NICE_SLIDER", "Slider::HORIZONTAL")
   s = s.replace("FL_RESERVED_TYPE", "Widget::RESERVED_TYPE")
   return s

def processNamespace(s):
   # replace fltk names
   s = s.replace("FL/Fl_Menu_Button", "fltk/PopupMenu")
   s = s.replace("FL/Fl_Double_Window", "fltk/Window")
   s = s.replace("Fl_Menu_Button", "PopupMenu")
   s = s.replace("Fl_Double_Window", "Window")
   s = s.replace("FL/fl_show_input.H", "fltk/ask.h")
   s = s.replace("FL/fl_message.H", "fltk/ask.h")
   s = s.replace("EXPORT", "FL_API")
   s = s.replace("damage(D", "set_damage(D")
   s = s.replace("HORIZONTAL", "ScrollGroup::HORIZONTAL")
   s = s.replace("Boxtype", "Box*")
   
   # convert eg. Fl_Check_Button to Check_Button but not Key_Func
   s = re.sub("([^\"])Fl_([\w]*)([a-z])_([A-Z])", "\g<1>\g<2>\g<3>\g<4>", s)

   # specific exceptions to the above rule
   s = s.replace("Style_Table_Entry", "StyleTableEntry")
   
   # close inserted Rectangle's
   # (?s) causes . to also match newlines
   s = re.sub("(?s)(Rectangle\(.*?\));", "\g<1>);", s, 0)

   # mangle #include lines
   s = re.sub("# *include *<FL/Fl.H>", FL_INCLUDE, s)
   s = re.sub("# *include *<FL/Fl_Scrollbar.H>", SCROLL_INCLUDE, s)
   s = re.sub("# *include *<FL/Fl_", "#include <fltk/", s)
   s = re.sub("# *include *<FL/", "#include <fltk/", s)

   # cleanup bad conversions
   s = s.replace("fltk/drawtext", "fltk/draw")
   
   # (?m) causes ^ and $ to be match \n within a single string
   s = re.sub("(?m)^# *", "#", s)
   s = s.replace(".H", ".h")
   s = s.replace("Fl::", "fltk::")
   s = s.replace("<FL/", "<fltk/")
   s = s.replace("#include <fltk/Preferences.h>", "")

   # remove Fl prefixes that don't begin with double-quote
   # to prevent renaming user include files with this prefix
   s = re.sub("([^\"])fl_", "\g<1>", s)
   s = re.sub("([^\"])FL_", "\g<1>", s)
   s = re.sub("([^\"])Fl_", "\g<1>", s)

   return s

def processHeader(s):
   # insert using namespace fltk into all headers
   i = s.rfind("#include")
   if (i != -1):
      i = s.find("\n", i)
      s = s[:i]+"\n\nusing namespace fltk;"+s[i:]
   
   return s

def process(fileName):
   if (os.access(backupDir+"/"+fileName, os.R_OK) == 0):
      shutil.copyfile(fileName, backupDir+"/"+fileName)

   f = open(fileName, 'rw')
   s = f.read()

   # check if the file has already been processed
   if (s.endswith(codeComment)):
      return

   print fileName

   s = processHashDefines(s)
   s = processInlines(s)
   s = processKeyboard(s)   
   s = processEnumerations(s)
   s = processNamespace(s)

   if (fileName.endswith(".h") or
       fileName.endswith(".H")):
       s = processHeader(s)
  
   f.close()
   f = open(fileName, 'w')
   f.write(s)
   f.write(codeComment)
   f.close()   
   
def main():
   # create the backup directory if it doesn't already exist
   if (os.access(backupDir, os.R_OK|os.W_OK) == 0):
      os.mkdir(backupDir)
      
   if (len(sys.argv) == 1):
      # process all source files in current directory
      files = glob.glob("*.h")
      files.extend(glob.glob("*.H"))
      files.extend(glob.glob("*.cxx"))
      files.extend(glob.glob("*.cpp"))
      for i in range(len(files)):
         process(files[i])
   else:
      # process all source files specified on the command line
      for i in range(1, len(sys.argv)):
         process(sys.argv[i])

main()

#
# ChangeLog
#
# 0.2 - 
# * Only remove userscores from FL_xx_Xxx variables (not user variables)
# * Insert "using namespace fltk" after the pattern #ifndef xxx\n#define xxx\n
#   and only in header files. (In lieu of before the word 'class'
# * renamed "delete unused imports" to "delete unused header files"
# * fixed problem with refactor() skipping lines
# * handle more conversions found in FL/fl_draw.H
# * processHashDefines() is now invoked
# * conversions for FL_DARK_xxx colours
#
# 0.1 - October 2005
# * Initial version
#
# End of $Id: $"

