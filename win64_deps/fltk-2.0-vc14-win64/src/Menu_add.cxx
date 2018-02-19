//
// "$Id: Menu_add.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Menu utilities for the Fast Light Tool Kit (FLTK).
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

// Methods to create/destroy the items in an Menu widget.  Used to
// emulate XForms and to provide convienence functions for building
// menus and browsers.

// These functions are provided for fltk 1.0 compatability and probably
// should be avoided in new programs. For new programs either create the
// widgets directly, or use List to return menu or browser items out
// of your own data.

// Compatability with fltk 1.0 and with XForms is only partial!

#include <fltk/Menu.h>
#include <fltk/Item.h>
#include <fltk/ItemGroup.h>
#include <fltk/Divider.h>
#include <fltk/string.h>
#include <ctype.h>
#include "ARRAY.h"

using namespace fltk;

////////////////////////////////////////////////////////////////
// Menu-style api:

static Widget* append(
  Group* g,
  const char *label,
  int flags,
  int insert_here
) {
  Group* saved = Group::current();
  Group::current(0);
  Widget* o;
  if (flags & SUBMENU) {
    o = new ItemGroup();
  } else {
    o = new Item();
  }
  char buf[1024];
  const char *p;
  char *q;
  for (p = label, q = buf; *p; *q++ = *p++)
    if (*p == '\\' && p[1]) p++;
  *q = 0;

  o->copy_label(buf);
  if (flags & MENU_RADIO) o->type(Item::RADIO);
  else if (flags & MENU_TOGGLE) o->type(Item::TOGGLE);
  // Shift the old flags values over to where they are in fltk,
  // but also allow new fltk flag values (this was done so RAW_LABEL
  // could be put in there for flwm)
  o->set_flag(((flags<<8)&(INACTIVE|STATE|INVISIBLE))|(flags&~0x1ff));
  if (insert_here) g->insert(*o, insert_here-1);
  else g->add(o);
  if (flags & MENU_DIVIDER) {
    Widget* d = new Divider();
    if (insert_here) g->insert(*d, insert_here);
    else g->add(d);
  }
  Group::current(saved);
  return o;
}

// flags to determine what innards() does:
enum AddType {ADD=0, FIND=1, REPLACE=2};

// Skip @ commands, return pointer to null or first actual letter:
// Except we don't skip a trailing command with no semicolon.
static inline void skip_embedded(const char*& aa) {
  const char* a = aa;
  while (*a == '@') {
    if (*++a == '@') {aa = a; return;} // return pointer to second @ of @@
    for (;; a++) {
      if (!*a) return; // leave pointer unchanged for trailing command
      if (isspace((uchar)*a) || *a==';') {aa = ++a; break;}
      if (*a == '@') {aa = a; break;}
    }
  }
}

// See if widget has the same label, except ignore @-sequences or &-sequences
// when comparing. If the new label has any @-sequences, replace the old label.
static bool match_and_replace(Widget* i, const char* B, int flags, AddType what) {
  const char* b = B;
  const char* a = i->label();
  if (!a) return false;
  // Check for exact match (thus avoiding replacement if all @-commands are
  // the same as before):
  if (!strcmp(a,b)) return true;
  if (flags&RAW_LABEL) return false;
  bool replace = false;
  for (;;) {
    if (*a == '@') skip_embedded(a);
    if (*b == '@') {skip_embedded(b); replace = true;}
    if (*a == '&') a++;
    if (*b == '&') {b++; replace = true;}
    if (*a != *b) return false;
    if (!*a) {
      if (replace && (what==REPLACE)) i->copy_label(B);
      return true;
    }
    ++a; ++b;
  }
}

FL_API bool fl_menu_replaced; // hack so fluid can tell what replace() did

// Innards of Menu::add() and Menu::replace() methods:
static Widget* innards(
  Group* top,
  const char *label,
  unsigned shortcut,
  Callback *callback,
  void *data,
  int flags,
  AddType what,
  int insert_here
) {
  Group* group = top;
  int bufsize = strlen(label)+1;
  ARRAY(char, buf, bufsize);

  int flags1 = 0;
  const char* item_label;
  for (;;) {    /* do all the supermenus: */

    // leading slash makes us assumme it is a filename:
    if (*label == '/') {item_label = label; break;}

    // leading underscore causes divider line:
    if (*label=='_' || *label=='-') {label++; flags1 = MENU_DIVIDER;}

    // copy to buf, changing \x to x:
    char* q = buf;
    const char* p;
    for (p=label; *p && *p != '/'; *q++ = *p++) if (p[0]=='\\' && p[1]) p++;
    *q = 0;
    item_label = buf;

    // if not followed by slash it is not a menu title:
    if (*p != '/') break;

    // point at the next label:
    label = p+1;

    // find a matching menu title:
    for (int n = group->children();;) {
      if (!n) { // create a new menu
	if (what == FIND) {
	  // give up on hierarchy search and find flat item:
	  item_label = label;
	  group = top;
	  goto BREAK1;
	}
	group = (Group*)append(group, item_label, SUBMENU|flags1, 0);
	break;
      }
      Widget* w = group->child(--n);
      if (w->is_group() && match_and_replace(w, item_label, flags, what)) {
	group = (Group*)w;
	break;
      }
    }
    flags1 = 0;
  }
 BREAK1:

  Widget* item = 0;

  // names ending in '/' create an empty menu or return the menu title:
  if (!*item_label && (group != top || flags1)) {
    item = group;
    // hack so "/_" adds a divider line
    if (flags1 && what!=FIND) {
      Group* saved = Group::current();
      Group::current(0);
      group->add(new Divider());
      Group::current(saved);
      return item;
    }

  } else {

    // find a matching menu item:
    if (what != ADD) for (int n = group->children(); n--;) {
      Widget* w = group->child(n);
      if (!w->is_group() && match_and_replace(w, item_label, flags, what)) {
	item = w;
	break;
      }
    }
  }

  if (item) {
    if (what == FIND) return item;
    fl_menu_replaced = true;
  } else {
    if (what == FIND) return 0;
    item = append(group, item_label, flags|flags1, insert_here);
    fl_menu_replaced = false;
  }

  /* fill it in */
  item->shortcut(shortcut);
  if (callback) item->callback(callback);
  item->user_data(data);
  item->w(0); item->h(0); // force re-measure
  top->relayout();
  return item;
}

/*! Split label at '/' characters and add a hierachial Item.

    Adds new items and hierarchy to a Menu or Browser.

    \a label :
    The label is split at '/' characters to automatically
    produce submenus. The submenus are created if they do not
    exist yet, and a new Item widget is added to the end of it.

    A trailing '/' can be used to create an empty submenu (useful for
    forcing a certain ordering of menus before you know what items are
    in them).

    Backslashes in the string "quote" the next character, which allows
    you to put forward slashes into a menu item. Notice that in C++
    you must type "\\/" for a forward slash, and "\\\\" for a backward
    slash.

    \a shortcut : 0 for no shortcut, fltk::CTRL|'a' for ctrl-a, etc.

    \a callback : function to call when item picked. If null, the callback
    for the Menu widget itself is called.

    \a data : second argument passed to the callback.

    \a flags : useful flags are:
    - INACTIVE makes the item grayed out and unpickable
    - INVISIBLE makes it not visible to the user, as though it was
      not there. This is most useful for making extra shortcuts that do
      the same thing.
    - RAW_LABEL stops it from interpreting '&' and '@' characters
      in the label.

    Returns a pointer to the new Item. You can further modify it to
    get results that can't be gotten from these function arguments.
*/
Widget* Menu::add(
  const char *label,
  unsigned shortcut,
  Callback *callback,
  void *data,
  int flags
) {
  return ::innards(this,label,shortcut,callback,data,flags,ADD,0);
}

/*! Split label at '/' characters and add or replace a hierachial Item.

    This is what menu::add() did in fltk 1.

    Same rules as add() except if the item already exists it is changed
    to this new data, instead of a second item with the same label being
    created.

    If the \a label has any @ or & commands in any portion, that
    portion is relabelled, thus you can use this to change the
    appearance of existing menu items. However if the new label does
    not have any such commands, the label remains as before, with any
    older @-commands.
*/
Widget* Menu::replace(
  const char *label,
  unsigned shortcut,
  Callback *callback,
  void *data,
  int flags
) {
  return ::innards(this,label,shortcut,callback,data,flags,REPLACE,0);
}

/*! Split label at '/' characters and add a hierachial Item.

    Same rules as add() except the item is inserted at index \a n
    of the final menu. Use 0 to put it at the start. Any number
    larger or equal to the current item count adds the new item
    to the end.
*/
Widget* Menu::insert(
  int n,
  const char *label,
  unsigned shortcut,
  Callback *callback,
  void *data,
  int flags
) {
  return ::innards(this,label,shortcut,callback,data,flags,ADD,n+1);
}

/*! Return the item with the given label

    This searches both the top level for an exact match, and splits
    the label at '/' to find an item in a hierarchy. Thus it matches
    the strings passed to both the long and short forms of add().

    If the item is found a pointer to it is returned, otherwise NULL
    is returned.
*/
Widget* Menu::find(const char* label) const {
  return ::innards((Group*)this,label,0,0,0,0,FIND,0);
}

////////////////////////////////////////////////////////////////
// Browser-style api:

// Innards of Menu::add() and Menu::replace() methods:
// This is the same code except it does not do the splitting at '/'
// thus it is much simpler
static Widget* flat_innards(
  Group* group,
  const char *label,
  void *data,
  int flags,
  AddType what,
  int insert_here
) {
  Widget* item = 0;

  // find a matching menu item:
  if (what != ADD) for (int n = group->children(); n--;) {
    Widget* w = group->child(n);
    if (!w->is_group() && match_and_replace(w, label, flags, what)) {
      item = w;
      break;
    }
  }

  if (item) {
    if (what == FIND) return item;
    fl_menu_replaced = true;
  } else {
    if (what == FIND) return 0;
    item = append(group, label, flags, insert_here);
    fl_menu_replaced = false;
  }

  /* fill it in */
  item->user_data(data);
  group->relayout();
  return item;
}

/*! Create a new Item and add it to the top-level of the hierarchy.

    This matches add() from the Browser in fltk1.

    Unlike the add() with more arguments, this one does *not* split
    the label at '/' characters. The label is used unchanged.
*/
Widget* Menu::add(const char* label, void* data) {
  return ::flat_innards(this,label,data,0,ADD,0);
}

/*! Create a new Item and add it to the top-level of the hierarchy.

    This matches insert() from the Browser in fltk1, except the
    index is 1 less than before (first item is zero, not 1)

    Unlike the insert() with more arguments, this one does *not* split
    the label at '/' characters. The label is used unchanged.
*/
Widget* Menu::insert(int n, const char* label, void* data) {
  return ::flat_innards(this,label,data,0,ADD,n+1);
}

/*! Create or replace an Item at the top-level of the hierarchy.

    The top level is searched for an item that matches the given
    label. If found it's data is changed to the passed data, if
    not found a new Item is created and added at the end.
*/
Widget* Menu::replace(const char* label, void* data) {
  return ::flat_innards(this,label,data,0,REPLACE,0);
}

/*!
  Add a parent widget to a (possibly) lower level of the
  hierarchy, such as returned by add_group().
*/
Group* Menu::add_group(const char* label, Group* parent, void* data) {
  return (Group*)::flat_innards(parent?parent:this, label, data, SUBMENU, ADD, 0);
}

/*!
  Add a non-parent widget to a (possibly) lower level of the
  hierarchy, such as returned by add_group(). If parent is null
  or this then this is the same as add(label,data).
*/
Widget* Menu::add_leaf(const char* label, Group* parent, void* data) {
  return ::flat_innards(parent?parent:this, label, data, 0, ADD, 0);
}

/*! \fn void Menu::remove(const char* l)
  Does find(l) and then deletes that widget.
*/

////////////////////////////////////////////////////////////////

/*! This is a Forms (and SGI GL library) compatable add function, it
  adds many menu items, with '|' seperating the menu items, and tab
  seperating the menu item names from an optional shortcut string. */
Widget* Menu::add_many(const char *str) {
  int bufsize = strlen(str)+1;
  ARRAY(char, buf, bufsize);
  Widget* r = 0;
  while (*str) {
    const char* start = str;
    unsigned shortcut = 0;
    char *c;
    for (c = buf; *str && *str != '|'; str++) {
//    if (*str == '\t') {*c++ = 0; shortcut = old_shortcut(str);}
//    else
      *c++ = *str;
    }
    *c = 0;
    if (*str) {
      r = add(buf, shortcut, 0, 0, 0);
      str++;
    } else {
      r = add(start, shortcut, 0, 0, 0);
    }
  }
  return r;
}

//
// End of "$Id: Menu_add.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
