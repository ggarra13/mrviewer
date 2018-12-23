// "$Id:"
//
// Subclasses of FluidType for each type of Widget in FLTK.
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

// This file needs to be split up into important subclasses that are
// used by multiple files. Many of these subclasses are only used by
// the fluid_menus file and should just be imbedded in there.

#ifndef fltk_widget_type_h
#define fltk_widget_type_h

#include <fltk/Box.h>
#include <fltk/TabGroup.h>
#include <fltk/WizardGroup.h>
#include <fltk/PopupMenu.h>
#include <fltk/Choice.h>
#include <fltk/MenuBar.h>
#include <fltk/InputBrowser.h>
#include <fltk/FileBrowser.h>
#include <fltk/Divider.h>
#include <fltk/InvisibleBox.h>
#include <fltk/Button.h>
#include <fltk/ReturnButton.h>
#include <fltk/RepeatButton.h>
#include <fltk/LightButton.h>
#include <fltk/CheckButton.h>
#include <fltk/RadioButton.h>
#include <fltk/Clock.h>
#include <fltk/Adjuster.h>
#include <fltk/Dial.h>
#include <fltk/ThumbWheel.h>
#include <fltk/TextDisplay.h>
#include <fltk/TextEditor.h>
#include <fltk/FileInput.h>
#include <fltk/ProgressBar.h>
#include <fltk/Scrollbar.h>
#include <fltk/Output.h>
#include <fltk/ValueInput.h>
#include <fltk/ValueOutput.h>
#include <fltk/ValueSlider.h>
#include <fltk/BarGroup.h>
#include <fltk/StatusBarGroup.h>
#include <fltk/HelpView.h>
#include <fltk/Item.h>
#include "GroupType.h"

class InvisibleBoxType : public WidgetType {
 public:
  virtual const char *type_name() const {return "fltk::InvisibleBox";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::InvisibleBox(x,y,w,h,0);}
  WidgetType *_make() {return new InvisibleBoxType();}
  int pixmapID() { return 5; }
};

extern const Enumeration buttontype_menu[];
class ButtonType : public WidgetType {
  const Enumeration *subtypes() const {return buttontype_menu;}
 public:
  virtual const char *type_name() const {return "fltk::Button";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::Button(x,y,w,h,"button");}
  WidgetType *_make() {return new ButtonType();}
  int is_button() const {return 1;}
  int pixmapID() { return 2; }
};

class ReturnButtonType : public ButtonType {
 public:
  virtual const char *type_name() const {return "fltk::ReturnButton";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::ReturnButton(x,y,w,h,0);}
  WidgetType *_make() {return new ReturnButtonType();}
  int pixmapID() { return 23; }
};

class RepeatButtonType : public WidgetType {
 public:
  virtual const char *type_name() const {return "fltk::RepeatButton";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::RepeatButton(x,y,w,h,0);}
  WidgetType *_make() {return new RepeatButtonType();}
  int pixmapID() { return 25; }
};

class CheckButtonType : public ButtonType {
 public:
  virtual const char *type_name() const {return "fltk::CheckButton";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::CheckButton(x,y,w,h,"button");}
  WidgetType *_make() {return new CheckButtonType();}
  int pixmapID() { return 3; }
};

class LightButtonType : public ButtonType {
 public:
  virtual const char *type_name() const {return "fltk::LightButton";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::LightButton(x,y,w,h,"button");}
  WidgetType *_make() {return new LightButtonType();}
  int pixmapID() { return 24; }
};

class RadioButtonType : public ButtonType {
 public:
  virtual const char *type_name() const {return "fltk::RadioButton";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::RadioButton(x,y,w,h,"button");}
  WidgetType *_make() {return new RadioButtonType();}
  int pixmapID() { return  4; }
};

extern const Enumeration input_type_menu[] ;

class InputType : public WidgetType {
  int is_input() const {return 1;}
  const Enumeration *subtypes() const {return input_type_menu;}
 public:
  virtual const char *type_name() const {return "fltk::Input";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    fltk::Input *o = new fltk::Input(x,y,w,h,"input:");
    o->value("Text Input");
    return o;
  }
  WidgetType *_make() {return new InputType();}
  int pixmapID() { return  14; }
};

class PackType : public GroupType {
  //const Enumeration *subtypes() const {return pack_type_menu;}
 public:
  virtual const char *type_name() const {return "fltk::PackedGroup";}
  WidgetType *_make() {return new PackType();}
  int pixmapID() { return  22; }
};

class TabsType : public GroupType {
 public:
  virtual const char *type_name() const {return "fltk::TabGroup";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    fltk::TabGroup *g = new fltk::TabGroup(x,y,w,h); g->resizable(0);
    fltk::Group::current(0); return g;}
  WidgetType *_make() {return new TabsType();}
  FluidType* click_test(int,int);
  void add_child(FluidType*, FluidType*);
  void remove_child(FluidType*);
  // live mode functionalities
  fltk::Widget* enter_live_mode(int top);
  int pixmapID() { return 13; }
};

class WizardGroupType : public GroupType {
 public:
  virtual const char *type_name() const {return "fltk::WizardGroup";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::WizardGroup(x,y,w,h);}
  WidgetType *_make() {return new WizardGroupType();}
  // live mode functionalities
  int pixmapID() { return 21; }
};

extern const Enumeration scroll_type_menu[];

class ScrollType : public GroupType {
  const Enumeration *subtypes() const {return scroll_type_menu;}
 public:
  virtual const char *type_name() const {return "fltk::ScrollGroup";}
  WidgetType *_make() {return new ScrollType();}
  int pixmapID() { return 19; }
};

class TileType : public GroupType {
 public:
  virtual const char *type_name() const {return "fltk::TiledGroup";}
  WidgetType *_make() {return new TileType();}
  int pixmapID() { return 20; }
};

class MenuType : public GroupType {
 public:
  int is_group() const {return 0;}
  MenuType() : GroupType() {}
  ~MenuType() {}
  FluidType* click_test(int x, int y);
  int pixmapID() { return -1; }
};

extern const Enumeration button_type_menu[];

class PopupMenuType : public MenuType {
  const Enumeration *subtypes() const {return button_type_menu;}
 public:
  virtual const char *type_name() const {return "fltk::PopupMenu";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::PopupMenu(x,y,w,h,"menu");}
  WidgetType *_make() {return new PopupMenuType();}
  int pixmapID() { return 26; }
};

extern const Enumeration input_browser_type_menu[];

class InputBrowserType : public MenuType {
  int is_input_browser() const {return 1;}
  const Enumeration *subtypes() const {return input_browser_type_menu;}
 public:
  virtual const char *type_name() const {return "fltk::InputBrowser";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::InputBrowser(x,y,w,h);
  }
  WidgetType *_make() {return new InputBrowserType();}
  int pixmapID() { return 15; }
};

extern const Enumeration browser_type_menu[];

class FileBrowserType : public WidgetType {
  const Enumeration *subtypes() const {return browser_type_menu;}
 public:
  virtual const char *type_name() const {return "fltk::FileBrowser";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::FileBrowser(x,y,w,h);
  }
  WidgetType *_make() {return new FileBrowserType();}
  int pixmapID() { return 33; }
};

class MenuBarType : public MenuType {
 public:
  virtual const char *type_name() const {return "fltk::MenuBar";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::MenuBar(x,y,w,h);}
  WidgetType *_make() {return new MenuBarType();}
  int pixmapID() { return 17; }
};

extern const Enumeration item_type_menu[];
class ItemType : public WidgetType {
 public:
  const Enumeration* subtypes() const {return item_type_menu;}
  const char* type_name() const {return "fltk::Item";}
  int is_menu_item() const {return 1;}
  int is_button() const {return 1;} // this gets shortcut to work
  fltk::Widget* widget(int x,int y,int w,int h);
  WidgetType *_make() {return new ItemType();}
  int pixmapID() { return 16; }
};

class DividerType : public WidgetType {
 public:
  const Enumeration* subtypes() const {return 0;}
  const char* type_name() const {return "fltk::Divider";}
  int is_menu_item() const {return 1;}
  fltk::Widget* widget(int x,int y,int w,int h) { return new fltk::Divider; }
  WidgetType *_make() {return new DividerType();}
  int pixmapID() { return 16; }
};

class SubmenuType : public GroupType {
 public:
  const Enumeration* subtypes() const {return 0;}
  const char* type_name() const {return "fltk::ItemGroup";}
  int is_menu_item() const {return 1;}
  fltk::Widget* widget(int x,int y,int w,int h);
  WidgetType *_make() {return new SubmenuType();}
  int pixmapID() { return 18; }
};

class ChoiceType : public MenuType {
 public:
  virtual const char *type_name() const {return "fltk::Choice";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::Choice(x,y,w,h,"choice:");
  }
  WidgetType *_make() {return new ChoiceType();}
  int pixmapID() { return 15; }
};

class BrowserType : public MenuType {
  const Enumeration *subtypes() const {return browser_type_menu;}
 public:
  virtual const char *type_name() const {return "fltk::Browser";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::Browser(x,y,w,h);
  }
  WidgetType *_make() {return new BrowserType();}
  int pixmapID() { return 31; }
};

class ClockType : public WidgetType {
 public:
  virtual const char *type_name() const {return "fltk::Clock";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::Clock(x,y,w,h);}
  WidgetType *_make() {return new ClockType();}
  int pixmapID() { return 34; }
};

class AdjusterType : public WidgetType {
  int is_valuator() const {return 1;}
 public:
  virtual const char *type_name() const {return "fltk::Adjuster";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::Adjuster(x,y,w,h);}
  WidgetType *_make() {return new AdjusterType();}
  int pixmapID() { return 40; }
};

extern const Enumeration dial_type_menu[];

class DialType : public WidgetType {
  const Enumeration *subtypes() const {return dial_type_menu;}
  int is_valuator() const {return 1;}
 public:
  virtual const char *type_name() const {return "fltk::Dial";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::Dial(x,y,w,h);}
  WidgetType *_make() {return new DialType();}
  int pixmapID() { return 42; }
};

class ThumbWheelType : public WidgetType {
  int is_valuator() const {return 1;}
 public:
  virtual const char *type_name() const {return "fltk::ThumbWheel";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::ThumbWheel(x,y,w,h);
  }
  WidgetType *_make() {return new ThumbWheelType();}
  int pixmapID() { return 43; }
};

class TextDisplayType : public WidgetType {
  int textstuff(int w, fltk::Font* f, int& s, fltk::Color c);
 public:
  virtual void ideal_size(int &w, int &h) {
    fltk::TextDisplay *myo = (fltk::TextDisplay *)o;
    fltk::setfont(myo->textfont(), myo->textsize());
    h -= o->box()->dh();
    w -= o->box()->dw();
    int ww = (int) fltk::getwidth("m");
    w = ((w + ww - 1) / ww) * ww + o->box()->dw();
    h = (int) (((h + fltk::getascent() - 1) / fltk::getascent() ) * fltk::getascent() +
               o->box()->dh());
    if (h < 30) h = 30;
    if (w < 50) w = 50;
  }
  virtual const char *type_name() const {return "fltk::TextDisplay";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    fltk::TextDisplay *myo = new fltk::TextDisplay(x,y,w,h);
    myo->box(fltk::DOWN_BOX);
    return myo;
  }
  WidgetType *_make() {return new TextDisplayType();}
  int pixmapID() { return 28; }
};

class TextEditorType : public WidgetType {
  int textstuff(int w, fltk::Font* f, int& s, fltk::Color c);
 public:
  virtual void ideal_size(int &w, int &h) {
    fltk::TextEditor *myo = (fltk::TextEditor *)o;
    fltk::setfont(myo->textfont(), myo->textsize());
    h -= o->box()->dh();
    w -= o->box()->dw();
    int ww = (int)fltk::getwidth("m");
    w = ((w + ww - 1) / ww) * ww + o->box()->dw();
    h = (int) (((h + fltk::getascent() - 1) / fltk::getascent()) * fltk::getascent() +
               o->box()->dh());
    if (h < 30) h = 30;
    if (w < 50) w = 50;
  }
  virtual const char *type_name() const {return "fltk::TextEditor";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    fltk::TextEditor *myo = new fltk::TextEditor(x,y,w,h);
    return myo;
  }
  WidgetType *_make() {return new TextEditorType();}
  int pixmapID() { return 29; }
};

class FileInputType: public WidgetType {
  fltk::Item *subtypes() {return 0;}
  int textstuff(int w,fltk::Font* f, int& s, fltk::Color c);
 public:
  virtual void ideal_size(int &w, int &h) {
    fltk::FileInput *myo = (fltk::FileInput *)o;
    fltk::setfont(myo->textfont(), myo->textsize());
    h = (int) (fltk::getdescent() + myo->textsize() + 4);
    w -= o->box()->dw();
    int ww = (int)fltk::getwidth("m",1);
    w = ((w + ww - 1) / ww) * ww + o->box()->dw();
    if (h < 20) h = 20;
    if (w < 50) w = 50;
  }
  virtual const char *type_name() const {return "fltk::FileInput";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    fltk::FileInput *myo = new fltk::FileInput(x,y,w,h,"file:");
    myo->value("/now/is/the/time/for/a/filename.ext");
    return myo;
  }
  WidgetType *_make() {      return new FileInputType();  }
  int pixmapID() { return 30; }
};

class HelpViewType : public WidgetType {
 public:
  void ideal_size(int &w, int &h) {
    fltk::HelpView *myo = (fltk::HelpView *)o;
    fltk::setfont(myo->textfont(), (float) myo->textsize());
    h -= o->box()->dh();
    w -= o->box()->dw();
    int ww = (int) fltk::getwidth("m");
    w = ((w + ww - 1) / ww) * ww + o->box()->dw();
    h = (int) (((h + fltk::getascent() - 1) / fltk::getascent() ) * fltk::getascent() +
               o->box()->dh());
    if (h < 30) h = 30;
    if (w < 50) w = 50;
  }
  const char *type_name() const {return "fltk::HelpView";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    fltk::HelpView *myo = new fltk::HelpView(x,y,w,h);
    myo->value("<HTML><BODY><H1>HelpView Widget</H1>"
               "<P>This is a HelpView widget.</P></BODY></HTML>");
    return myo;}
  WidgetType *_make() {return new HelpViewType();}
  int pixmapID() { return 35; }
};

class ProgressBarType : public WidgetType {
 public:
  virtual const char *type_name() const { return "fltk::ProgressBar"; }
  fltk::Widget* widget(int x, int y, int w, int h) {
    return new fltk::ProgressBar(x, y, w, h);
  }
  WidgetType *_make() { return new ProgressBarType(); }
  int pixmapID() { return 36; }
};

extern const Enumeration slider_type_menu[];

class SliderType : public WidgetType {
  const Enumeration *subtypes() const {return slider_type_menu;}
  int is_valuator() const {return 2;}
 public:
  virtual const char *type_name() const {return "fltk::Slider";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::Slider(x,y,w,h);
  }
  WidgetType *_make() {return new SliderType();}
  int pixmapID() { return 37; }
};

class ScrollbarType : public SliderType {
  const Enumeration *subtypes() const {return 0;}
 public:
  virtual const char *type_name() const {return "fltk::Scrollbar";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::Scrollbar(x,y,w,h);
  }
  WidgetType *_make() {return new ScrollbarType();}
  int pixmapID() { return 19; }
};

extern const Enumeration output_type_menu[];
class OutputType : public InputType {
  const Enumeration *subtypes() const {return output_type_menu;}
 public:
  virtual const char *type_name() const {return "fltk::Output";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    fltk::Output *o = new fltk::Output(x,y,w,h,"output:");
    o->value("Text Output");
    return o;
  }
  WidgetType *_make() {return new OutputType();}
};

class ValueInputType : public WidgetType {
 public:
  virtual const char *type_name() const {return "fltk::ValueInput";}
  int is_valuator() const {return 1;}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::ValueInput(x,y,w,h,"value:");
  }
  WidgetType *_make() {return new ValueInputType();}
  int pixmapID() { return 44; }
};

class ValueOutputType : public WidgetType {
 public:
  virtual const char *type_name() const {return "fltk::ValueOutput";}
  int is_valuator() const {return 1;}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::ValueOutput(x,y,w,h,"value:");
  }
  WidgetType *_make() {return new ValueOutputType();}
  int pixmapID() { return 45; }
};

class ValueSliderType : public SliderType {
 public:
  virtual const char *type_name() const {return "fltk::ValueSlider";}
  fltk::Widget* widget(int x,int y,int w,int h) {
    return new fltk::ValueSlider(x,y,w,h);
  }
  WidgetType *_make() {return new ValueSliderType();}
  int pixmapID() { return 39; }
};

class BarGroupType : public WidgetType {
 public:
  virtual const char *type_name() const { return "fltk::BarGroup"; }
  fltk::Widget* widget(int x, int y, int w, int h) {
    return new fltk::BarGroup(x, y, w, h);
  }
  WidgetType *_make() { return new BarGroupType(); }
  int pixmapID() { return 17; }
};

class StatusBarGroupType : public WidgetType {
 public:
  virtual const char *type_name() const { return "fltk::StatusBarGroup"; }
  fltk::Widget* widget(int x, int y, int w, int h) {
    return new fltk::StatusBarGroup(x, y, w, h);
  }
  WidgetType *_make() { return new StatusBarGroupType(); }
  int pixmapID() { return 5; } // no nice bitmap yet use the WidgetType one
};

#endif
// End of "$Id"
