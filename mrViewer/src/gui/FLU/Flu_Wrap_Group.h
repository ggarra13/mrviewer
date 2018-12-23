// $Id: Flu_Wrap_Group.h,v 1.10 2004/01/27 21:44:21 jbryan Exp $

/***************************************************************
 *                FLU - FLTK Utility Widgets 
 *  Copyright (C) 2002 Ohio Supercomputer Center, Ohio State University
 *
 * This file and its content is protected by a software license.
 * You should have received a copy of this license with this file.
 * If not, please contact the Ohio Supercomputer Center immediately:
 * Attn: Jason Bryan Re: FLU 1224 Kinnear Rd, Columbus, Ohio 43212
 * 
 ***************************************************************/



#ifndef _FLU_WRAP_GROUP_H
#define _FLU_WRAP_GROUP_H

/* fltk includes */
#if 0
#  include <fltk/Group.h>
#  include <fltk/Scrollbar.h>
#else
#  include <fltk/ScrollGroup.h>
#endif

#include "FLU/Flu_Enumerations.h"

/*! 
  This class provides an alternative to fltk::Group that automatically
  arranges the children either left to right and top to bottom
  (for type() == \c fltk::VERTICAL), or top to bottom and left to right 
  (for type() == \c fltk::HORIZONTAL), within the available size of the
  group, with a scrollbar turning on if they don't all fit.
*/

/*! 
  This class is a group with two scrollbars and an \b fltk::Group inside 
  (both publicly exposed). The \b fltk::Group contains the actual child
  widgets of this group.

  Most of the \b fltk::Group member functions are reimplemented here in a 
  pass-through fashion to the internal group. This means that casual use of
  a descendent instance will be almost exactly the same as for a regular 
  \b fltk::Group, with any additional access provided directly 
  through member \b group.

  The goal of this class is to provide a group that dynamically and 
  evenly distributes its children within a fixed space, similar to those 
  available in other GUI toolkits.
*/
class FLU_EXPORT Flu_Wrap_Group : public fltk::ScrollGroup
{

 public:
  //! Normal FLTK constructor
  Flu_Wrap_Group( int x, int y, int w, int h, const char *l = 0 );

  //! \return the widget that is visibly above \b w in the group, or 
  //! \c NULL if no such widget exists
  fltk::Widget *above( const fltk::Widget* w ) const;

  //! \return the widget that is visibly below \b w in the group, or 
  //! \c NULL if no such widget exists
  fltk::Widget *below( const fltk::Widget* w ) const;

  //! \return the widget that is visibly to the left of \b w in the group, 
  //! or \c NULL if no such widget exists
  fltk::Widget *left( const fltk::Widget* w ) const;

  //! \return the widget that is visibly to the right of \b w in the group, 
  //! or \c NULL if no such widget exists
  fltk::Widget *right( const fltk::Widget* w ) const;

  //! \return the widget that logically follows the passed widget.
  fltk::Widget *next( const fltk::Widget* w ) const;

  //! \return the widget that logically comes before the passed widget.
  fltk::Widget *previous( const fltk::Widget* w ) const;

  //! Set the offset for where the first child starts
  inline void offset( int x, int y )
    { _offset[0] = x, _offset[1] = y; relayout(); }

  //! \return the x offset for where the first child starts
  inline int offset_x() const
    { return _offset[0]; }

  //! \return the y offset for where the first child starts
  inline int offset_y() const
    { return _offset[1]; }  

  //! Set the spacing between children
  inline void spacing( int x, int y )
    { _spacing[0] = x, _spacing[1] = y; relayout(); }

  //! Scroll the group to the beginning of the list
  void scroll_to_beginning();

  //! Scroll the group to the end of the list
  void scroll_to_end();

  //! Scroll the group so that the given widget is shown (usually aligned to
  //! the left/top)
  void scroll_to( const fltk::Widget* w );

  //! Scroll the group so that the given widget is shown (usually aligned to 
  //! the left/top)
  inline void scroll_to( const fltk::Widget& w )
    { scroll_to( &w ); }

  virtual int handle( int event );

  virtual void layout();

  virtual void draw();

  //! Returns the size of the grid in terms of rows and columns
  void grid( unsigned int& cols, unsigned int& rows ) const;

  //! Finds the column and row index of a widget
  void grid_find( unsigned int& col, unsigned int& row, 
		  const fltk::Widget* w ) const;

 protected:
  void _measure( int& maxW, int& maxH ) const;
  //! Finds the column and row index of a widget
  void grid_find( unsigned int& col, unsigned int& row,
		  const unsigned int cols, const unsigned int rows,
		  const fltk::Widget* w ) const;
  unsigned int index_from_grid( const unsigned int c, const unsigned int r,
				const unsigned int cols, 
				const unsigned int rows ) const;
  void layout_grid();

  const fltk::Widget* scrollToElement;
  int _offset[2], _spacing[2], _type;
};

#endif
