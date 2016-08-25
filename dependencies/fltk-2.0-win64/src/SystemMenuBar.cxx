//
// "$Id:$"
//
// MacOS system menu bar widget for the Fast Light Tool Kit (FLTK2).
//
// Copyright 1998-2005 by Bill Spitzak and others.
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
//     http://www.fltk.org/str.php
//

/**
 * This code is a quick hack! It was written as a proof of concept.
 * It has been tested on the "menubar" sample program and provides
 * basic functionality. 
 * 
 * To use the SystemMenuBar, simply replace the main fltk::MenuBar
 * in an application with fltk::SystemMenuBar and call fltk::System
 * MenuBar::layout() to refresh the menu in the top bar 
 *
 * FLTK features not supported by the Mac System menu
 *
 * - no invisible menu items
 * - no sybolic labels
 * - embossed labels will be underlined instead
 * - no font sizes
 * - Shortcut Characters should be English alphanumeric only, no modifiers yet
 * - no disable main menus
 * - changes to menubar in run-time don't update! 
 *     (disable, etc. - toggle and readio button do!)
 *
 * No care was taken to clean up the menu bar after destruction!
 * Many calls of the parent class don't work as expected.
 * Changing the menu items has no effect on the menu bar.
 */

#if defined(__APPLE__)

#include <fltk/osx.h>
#include <fltk/events.h>
#include <fltk/SystemMenuBar.h>
#include <fltk/Font.h>
#include <fltk/Item.h>
#include <fltk/ItemGroup.h>

#include <fltk/string.h>

using namespace fltk;

/**
 * copy the text of a menuitem into a buffer.
 * Skip all '&' which would mark the shortcut in FLTK
 * Skip all Mac control characters ('(', '<', ';', '^', '!' )
 */
static void catMenuText( const char *src, char *dst )
{
  char c;
  while ( *dst ) 
    dst++;
  if (src) {
    if ( *src == '-' ) 
      src++;
    while ( ( c = *src++ ) ) 
    {
      if ( !strchr( "&(<;^!", c )  )
        *dst++ = c;
    }
  }
  *dst = 0;
}

/**
 * append a marker to identify the menu font style
 * <B, I, U, O, and S
 */
static void catMenuFont( const Widget *m, char *dst )
{
  if ( !m->labeltype() && !m->labelfont() ) 
    return;
  while ( *dst ) 
    dst++;
    
  if ( m->labelfont() == m->labelfont()->bold() )
    strcat( dst, "<B" );
  if ( m->labelfont() == m->labelfont()->italic() )
    strcat( dst, "<I" );
  //if ( m->labelfont_ & FL_UNDERLINE )
  //  strcat( dst, "<U" );
  
  //if ( m->labeltype_ == FL_EMBOSSED_LABEL )
  //    strcat( dst, "<U" );
  //else if ( m->labeltype_ == FL_ENGRAVED_LABEL )
  //    strcat( dst, "<O" );
 // else if ( m->labeltype_ == FL_SHADOW_LABEL )
 //     strcat( dst, "<S" );
  //else if ( m->labeltype_ == FL_SYMBOL_LABEL )
      ; // not supported
}

static void setMenuShortcut( MenuHandle mh, int miCnt, const Widget *m )
{
  if ( !m->shortcut() ) 
    return;
  if ( m->is_group() )
    return;

  char key = m->shortcut() & 0xff;
  if ( !isalnum( key ) )
    return;
 
  long macMod = kMenuNoCommandModifier;
  if ( m->shortcut() & META ) macMod = kMenuNoModifiers;
  if ( m->shortcut() & SHIFT || isupper(key) ) macMod |= kMenuShiftModifier;
  if ( m->shortcut() & ALT ) macMod |= kMenuOptionModifier;
  if ( m->shortcut() & CTRL ) macMod |= kMenuControlModifier;
 
  //SetMenuItemKeyGlyph( mh, miCnt, key );
  SetItemCmd( mh, miCnt, toupper(key) );
  SetMenuItemModifiers( mh, miCnt, macMod );
}

static void setMenuFlags( MenuHandle mh, int miCnt, const Item *m )
{
  if ( m->type()==Item::TOGGLE )
    SetItemMark( mh, miCnt, m->state() ? 0x12 : 0 );
  else if ( m->type()==Item::RADIO )
    SetItemMark( mh, miCnt, m->state() ? 0x13 : 0 );
}

static void catMenuFlags( const Widget *m, char *dst )
{
  if (m->active()) 
    return;
  while (*dst) 
    dst++;
  strcat( dst, "(" );
}

/**
 * create a sub menu for a specific menu handle
 */
static void createSubMenu( MenuHandle mh, int &cnt, Group *mg )
{
  char buf[255];
  int miCnt = 1;
  int nChildren = mg->children();
  for (int i=0; i<nChildren; i++)
  {
    Widget *mm = mg->child(i);
    if (!mm->label()) continue;
    if (!mm->visible()) continue;
    MenuHandle smh = 0;
    buf[1] = 0;
    catMenuFont( mm, buf+1 );
    //catMenuShortcut( mm, buf+1 );
    catMenuText( mm->label(), buf+1 );
    catMenuFlags( mm, buf+1 );
    if ( mm->is_group() )
    {
      cnt++;
      smh = NewMenu( cnt, (unsigned char*)"\001 " );
      sprintf( buf+1+strlen(buf+1), "/\033!%c", cnt );
    }
    if (i<nChildren-1 && !mg->child(i+1)->label()) // DIVIDER hack
      strcat( buf+1, ";-" );
    buf[0] = strlen( buf+1 );
    AppendMenu( mh, (unsigned char*)buf );
    // insert Appearanc manager functions here!
    if (!mm->is_group())
      setMenuFlags( mh, miCnt, (Item*)mm );
    setMenuShortcut( mh, miCnt, mm );
    SetMenuItemRefCon( mh, miCnt, (UInt32)mm );
    miCnt++;
    if (i<nChildren-1 && !mg->child(i+1)->label()) // DIVIDER hack
      miCnt++;
    if ( mm->is_group() )
      createSubMenu( smh, cnt, (Group*)mm );
  }
  InsertMenu( mh, -1 );
}
 
void SystemMenuBar::layout() 
{
  open_display();
  char buf[255];
  int cnt = 1;

  system_menu_bar = this;
  
  int nMain = children();
  for (int i=0; i<nMain; i++) {
    Widget *mm = child(i);
    if (!mm->visible()) continue;
    buf[1] = 0;
    catMenuText(mm->label(), buf+1);
    buf[0] = strlen(buf+1);
    MenuHandle mh = NewMenu(++cnt, (unsigned char*)buf);
    if (mm->is_group())
      createSubMenu(mh, cnt, (Group*)mm);
    InsertMenu(mh, 0);
    if (!mm->active())
      DisableMenuItem(mh, 0);
  }
  DrawMenuBar();
}

void SystemMenuBar::draw() 
{
  // intentionally left blank
}


#endif /* __APPLE__ */

//
// End of "$Id:$".
//
