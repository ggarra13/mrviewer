//
// "$Id: InputBrowser.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// MicroSoft style "ComboBox" with the menu appearing below with a
// scrollbar. I would like to use the name "ComboBox" or "InputChoice"
// for a more user-friendly version which uses pop-up menus and
// positions the menu with the cursor pointing at the current item,
// but this version can be used to get what MicroSoft users expect.
// The implementation is a good example of how to get a widget to appear
// in a modal pop-up window.
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

#ifndef fltk_InputBrowser_h
#define fltk_InputBrowser_h

#include "Menu.h"
#include "Input.h"

namespace fltk {

class ComboBrowser;
class ComboWindow;

class FL_API InputBrowser : public Menu {
public:
  enum { // values for type()
    NORMAL = 0,
    NONEDITABLE = 1,
    INDENTED = 2,
    NONEDITABLE_INDENTED = 3
  };

  InputBrowser(int,int,int,int,const char * = 0);
  ~InputBrowser();
  static NamedStyle* default_style;

  void popup();
  void hide_popup();
  virtual int popup(int x, int y, int w, int h) { InputBrowser::popup(); return Menu::popup(Rectangle(x,y,w,h)); }

  virtual int handle(int);

  Widget* item() const ;
  Widget* item(Widget* v) const ;

  void minw(int i) { minw_ = i; }
  void minh(int i) { minh_ = i; }
  int minw() { return minw_; }
  int minh() { return minh_; }

  void maxw(int i) { maxw_ = i; }
  void maxh(int i) { maxh_ = i; }
  int maxw() { return maxw_; }
  int maxh() { return maxh_; }

  void text(const char *v) { m_input.text(v); }
  const char *text() const { return m_input.text(); }

protected:
  virtual void draw();
  static void input_cb(Input *w, InputBrowser *ib);

  Input m_input;

  ComboWindow *win;
  ComboBrowser *list;

  friend class ComboWindow;
  friend class ComboBrowser;

  int minw_, minh_, maxw_, maxh_;
  int over_now, over_last;
};

}

#endif

//
// End of "$Id: InputBrowser.h 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
