// "$Id: Menu.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
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

#include <fltk/events.h>
#include <fltk/Menu.h>
#include <fltk/damage.h>
#include <fltk/Item.h> // for TOGGLE, RADIO

#define checkmark(item) (item->type()>=Item::TOGGLE && item->type()<=Item::RADIO)

using namespace fltk;

////////////////////////////////////////////////////////////////

/*! \class fltk::List

  Allows a Browser or Choice or other subclass of Menu to display a
  hierarchy of data that is managed by the application rather than
  FLTK.

  This is done by making a subclass of List which creats a "dummy" widget,
  typically a subclass of Item, that describes a particular item that
  the browser or menu should display. Only one item is examined at any
  time and thus the dummy widget can be reused, so there is very
  little space overhead.

  This is designed for data formats where finding the Nth child of a
  parent is a very quick operation, ie an array. If your data is a
  list you can search it, the performance is probably acceptable for
  small lists with less than a hundred or so items. For a
  bidirectional list it may be useful to cache the last request and do
  a relative search, a Browser and Menu will usually ask for adjoining
  items.

  If you wish to use a MultiBrowser you must also have space in your
  data to store the state of the fltk::SELECTED flag on each item, and
  and must implement the flags_changed() method.

  If you wish to make a hierarcial Browser, you may want to have space
  in your data to store the state of the fltk::STATE flag on each parent
  item, and implement the flags_changed() method. If you don't do this
  the browser is only able to keep one item open at each level.

  The base List class returns the child widgets from the Menu that
  owns it.  All Menus share a single instance of this by default, so
  the default behavior is that child widgets appear as items in the
  menu or browser. Subclasses of List may want to call the base class
  to allow normal widgets to be prepended to whatever they return.
*/

/*!
  Return how many children are under a given item. If level is
  zero, this should return how many items are at the top
  level. Otherwise indexes is an array of level numbers indicating the
  index of an item at the top level, the index of the an item that is
  the child of that, and so on.

  This should return -1 if the item is not a "parent" item or the
  index array is illegal. It is not necessary to return the correct
  value until the parent is "open", which means the fltk::STATE flag
  was set in it, so if it is expensive to calculate the number you can
  return 1 for any closed parent.

  Here is a sample implementation, where Node is a data type that you
  have defined:

\code
int My_List::children(const fltk::Menu*, const int* indexes, int level) {
  Node* node = root;
  for (int l = 0; l < level; l++) {
    if (indexes[l] >= node->children_count()) return -1;
    node = node->child(indexes[l]);
    if (!node->is_parent()) return -1;
  }
  return node->children_count();
}
\endcode
*/
int List::children(const Menu* menu, const int* indexes, int level) {
  Group* group = (Group*)menu;
  while (level--) {
    int i = *indexes++;
    // browser can return indexes pointing on -1 values so we must check this :
    if (i < 0) return -1;    //if (i >= group->children()) return -1;
    Widget* widget = group->child(i);
    if (!widget->is_group()) return -1;
    group = (Group*)widget;
  }
  return group->children();
}

/*!
  Return a given child as a widget. draw() and measure() will be
  called on this widget to figure out where to place it and to draw
  it. Typical implementations create a reusable fltk::Item and fill it
  in with the correct data. This should return NULL if there is
  anything illegal about the indexes.

  Here is a sample implementation, where Node is a data type that you
  have defined. This demonstrates how to create the dummy widget:

\code
fltk::Widget* My_List::child(const fltk::Menu*, const int* indexes, int level) {
  Node* node = root;
  for (int l = 0; l <= level; l++) {
    if (!node->is_parent()) return 0;
    if (indexes[l] >= node->children_count()) return 0;
    node = node->child(indexes[l]);
  }
  static fltk::Widget* widget;
  if (!widget) {
    fltk::Group::current(0);
    widget = new fltk::Item();
  }
  widget->label(node->text());
  widget->w(0); // cause measure() to be called
  widget->user_data(node);
  if (node->selected) widget->set_flag(fltk::SELECTED);
  else widget->clear_flag(fltk::SELECTED);
  if (node->is_parent() && node->open) widget->set_flag(fltk::STATE);
  else widget->clear_flag(fltk::STATE);
  return widget;
}
\endcode
*/
Widget* List::child(const Menu* menu, const int* indexes,int level) {
  Group* group = (Group*)menu;
  for (;;) {
    int i = *indexes++;
    if (i < 0 || i >= group->children()) return 0;
    Widget* widget = group->child(i);
    if (!level--) return widget;
    if (!widget->is_group()) return 0;
    group = (Group*)widget;
  }
}

/*!
  This is called if the browser changes any flags on a widget, so that
  you can copy the values to permanent storage, and perhaps change
  other displays of the selection.

  Currently only the fltk::OPENED and fltk::SELECTED flags are ever changed.

  Here is a sample implementation, where Node is a data type that you
  have defined:
\code
void My_List::flags_changed(const fltk::Menu*, fltk::Widget* widget) {
  Node* node = (Node*)(widget->user_data());
  node->open = widget->flag(fltk::OPENED);
  node->selected = widget->flag(fltk::SELECTED);
}
\endcode
*/
void List::flags_changed(const Menu*, Widget*) {}

/*!
  The destructor does nothing. It is mostly here to shut up compiler
  warnings, and to allow subclasses that you want to dynamically
  allocate to work.

  Unfortunately C++ calls the destructor for static objects on program
  exit. I consider this a mistake, particularily because the
  destructor cannot assumme other objects (like the display
  connection) still exist.  It also slows down program exit. As all
  useable systems can harvest resources used by crashed programs,
  calling these destructors serve no useful purpose.
*/
List::~List() {}

static List default_list;

////////////////////////////////////////////////////////////////

/*! \class fltk::Menu

  The Menu base class is used by Browser, Choice, MenuBar, PopupMenu,
  ComboBox, and other widgets.  It is simply a Group and each item is
  a child Widget, but it provides functions to select and identify one
  of the widgets in the hierarchy below it and do that widget's
  callback directly, and functions to create and add Item and
  ItemGroup widgets to a hierarchy.

  A Menu can take a pointer to a List object, which allows the
  user program to dynamically provide the items as they are
  needed. This is much easier than trying to maintain an array of
  Widgets in parallel with your own data structures.

  It also provides several convienence functions for creating,
  rearranging, and deleting child Item and ItemGroup widgets.
*/

static void revert(Style *s) {
  s->box_ = FLAT_BOX;
  s->color_ = GRAY75;
  s->leading_ = 4;
}
static NamedStyle style("Menu", revert, &Menu::default_style);
/*! This style is assigned to Menu subclasses.

    Because of the need to be compatable with Windows, which has a
    somewhat inconsistent appearance, the default is different than
    Widget::default_style, and the usage of the fields is somewhat
    unusual:

    - The color(), textfont(), textsize(), textcolor() are
      used to draw each Item in the menu, unless the items set
      their own preferences. box() defaults to FLAT_BOX and color()
      defaults to GRAY75 to replicate Windows.
    - The popup menu window itself uses the box() that is stored
      in MenuWindow::default_style. It is not possible (or at least
      not easy) to make different popups have different borders.
    - However that box is colored by color(). Thus you can make
      the color different for each popup menu. This will only be
      visible if the items have transparent boxes.
    - The leading() is used to space apart the menu items. It
      defaults to 4 to replicate Windows.
    - buttonbox(), buttoncolor(), labelfont(), labelsize() and
      labelcolor() are \e not used. They can be used to draw the
      actual widget.

    The Browser subclass sets the style back to
    Widget::default_style, so that they appear more like a text editor
    (ie they put the color and leading back to normal).
*/
NamedStyle* Menu::default_style = &::style;

Menu::Menu(int x,int y,int w, int h,const char* l, bool begin)
  : Group(x,y,w,h,l,begin), list_(&default_list), item_(0) {
  resizable(0);
  callback(default_callback);
  style(default_style);
}

/*! \fn void Menu::list(List* list)
  Set the List that generates widgets for the menu. By default this
  is a dummy List that returns the child widgets of the Menu.
*/

/*! Calls list()->children(this, indexes, level) */
int Menu::children(const int* indexes, int level) const {
  return list_->children(this, indexes, level);
}

/*!
  Returns the number of children at the top level. Same as children(0,0).

  <i>This Overrides the method of the same name on fltk::Group</i>. This is
  so that an fltk::List can be used. However if no fltk::List is
  specified the action is identical to fltk::Group::children().
*/
int Menu::children() const {
  return list_->children(this, 0, 0);
}

/*! Returns the number of children of some child. Same as children(&i,1). */
int Menu::children(int i) const {
  return list_->children(this, &i, 1);
}

/*! Calls list()->child(this, indexes, level).
  If an fltk::List is used, the returned widget may be a temporary data
  structure and may be overwritten by another call to child() in this
  <i>or any other Menu</i>!
*/
Widget* Menu::child(const int* indexes, int level) const {
  return list_->child(this, indexes, level);
}

/*! Returns the given top-level child. Same as child(&index,0).

  <i>This Overrides the method of the same name on fltk::Group</i>. This is
  so that an fltk::List can be used. However if no fltk::List is
  specified the action is identical to fltk::Group::child(index).
*/
Widget* Menu::child(int n) const {
  return list_->child(this, &n, 0);
}

////////////////////////////////////////////////////////////////

/*! \fn Widget* Menu::item() const

  The "current item". In callbacks this is the item the user clicked
  on. Otherwise you probably can't make any assumptions about it's
  value.

  Browser::goto_index() sets this to the current item.

  Since this may be the result of calling child() the returned
  structure may be short-lived if an fltk::List is used.
*/

/*! \fn Widget* Menu::item(Widget* v)
  You can set item() with the second call, useful for outwitting the
  callback. This does not produce any visible change for the user.
*/

/*!
  Calls do_callback(). First it sets item() to the given widget, so
  the callback code can see it.

  Notice that this calls the callback on the Menu widget itself, not
  on the menu item. However the default callback for Menu widget does
  item()->do_callback() so by default the callback for each menu item
  is done.

  Callbacks for items can be disabled, so item()->when(WHEN_NEVER) will
  disable it for named item, but calling when(WHEN_NEVER) with menu instance
  will disable callbacks for all menu items (but not for the menu itself).
*/
void Menu::execute(Widget* widget) {
  item(widget);
  if(when() == WHEN_NEVER || item()->when() == WHEN_NEVER) return;
  if (widget) do_callback();
}

/*! The default callback for Menu calls item()->do_callback() but
  if user_data() is not null it is used instead of the item's user_data().
*/
void Menu::default_callback(Widget* widget, void* data) {
  Widget* item = ((Menu*)widget)->item();
  if (item) item->do_callback(item,
	item->user_data() ? item->user_data() : widget->user_data());
}

/*! Does nothing. This avoids wasting time measuring all the menu items. */
void Menu::layout() {layout_damage(0);}

////////////////////////////////////////////////////////////////

/*!
  Remembers a currently selected item in a hierarchy by setting the
  focus_index() of each group to point to the next one. The widget
  can then be recovered with get_item(). A redraw(DAMAGE_VALUE) is
  done so pulldown menus redraw their display.
*/
bool Menu::set_item(const int* indexes, int level) {
  int i = indexes[0];
  bool ret = false;
  if (focus_index() != i) {focus_index(i); ret = true;}
  if (i < 0 || i >= children()) {
    item(0);
  } else {
    item(child(i));
    int j = 1;
    while (item() && item()->is_group()) {
      Group* group = (Group*)item();
      int i = (j > level) ? -1 : indexes[j++];
      if (group->focus_index() != i) {group->focus_index(i); ret = true;}
      if (i < 0 || i >= group->children()) break;
      item(group->child(i));
    }
  }
  if (ret) redraw(DAMAGE_VALUE);
  return ret;
}

/*!
  Sets and returns item() based on the focus_index() in this and each
  child group, thus restoring the value saved with set_item().

  This either returns a non-group node, or child group that has an
  illegal Group::focus_index(), or null if this focus_index() is
  illegal.

  If an fltk::List is used this will probably only go to the first
  child and not descend any further.
*/
Widget* Menu::get_item() {
  int i = focus_index();
  if (i < 0 || i >= children()) {item(0); return 0;}
  item(child(i));
  while (item()->is_group()) {
    Group* group = (Group*)item();
    int i = group->focus_index();
    if (i < 0 || i >= group->children()) break;
    item(group->child(i));
  }
  return item();
}

/*!
  Convienence function to do set_item() when there is only one level
  of hierarchy. In this case you can consider the menu items to be
  indexes starting at zero.
*/
bool Menu::value(int v) {
  if (v == focus_index()) return false;
  focus_index(v);
  redraw(DAMAGE_VALUE);
  return true;
}

////////////////////////////////////////////////////////////////

/*! Recursive innards of handle_shortcut. The recursive part only works
  for real children of a child Group, the hierarchy in a List is ignored.
  This is because generating every item in a list could take a very
  long time, possibly forever. */
static Widget* shortcut_search(Group* g) {
  for (int i = 0; i < g->children(); i++) {
    Widget* item = g->child(i);
    if (!item->active()) continue;
    if (item->test_shortcut(false)) {
      g->focus_index(i);
      return item;
    }
    if (item->is_group()) {
      item = shortcut_search((Group*)item);
      if (item) {
	g->focus_index(i);
	return item;
      }
    }
  }
  return 0;
}

/*!
  Respond to the current fltk::SHORTCUT or fltk::KEY event by finding
  a menu item it matches and calling execute() on it. True is returned
  if a menu item is found, false if none. Items are searched for a
  matching shortcut() value. "&x" shortcuts are ignored, they are
  used only for navigating when the menu is visible.

  If you use a List, only the top-most level items are searched for
  shortcuts. Recursion is done only if the children are Group widgets,
  and then only the actual children of that Group (not any List it may
  contain if it is another Menu) are searched. This is necessary
  because some Lists are infinite in size, and usually they don't have
  shortcuts anyway.

  This will return invisible (but active) items, even though it is
  impossible to select these items with the gui. This is done so that
  more than one shortcut for an action may be given by putting
  multiple copies of the item in, where only the first is visible.
*/
int Menu::handle_shortcut() {
  //if (event_key_repeated()) return 0; // ignore repeating keys
  int children = this->children();
  for (int i = 0; i < children; i++) {
    Widget* item = child(i);
    if (!item->active()) continue;
    if (item->test_shortcut(false)) {
      value(i);
      if (checkmark(item))
        item->invert_flag(STATE);

      execute(item);
      return 1;
    }
    if (item->is_group()) {
      item = shortcut_search((Group*)item);
      if (item) {
	value(i);
        if (checkmark(item))
          item->invert_flag(STATE);

        execute(item);
        return 1;
      }
    }
  }
  return 0;
}

/*! \fn int Menu::size() const
  Returns children() (for back compatability with older versions of fltk).
*/

//
// End of "$Id: Menu.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
