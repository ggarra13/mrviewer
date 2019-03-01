// $Id: GridGroup.h,v 1.10 2004/01/27 21:44:21 jbryan Exp $

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



#ifndef _GRIDGROUP_H
#define _GRIDGROUP_H

/* fltk includes */
#include <FL/Fl_Group.H>

#include <FLU/Flu_Enumerations.h>

//! This class provides an alternative to Fl_Group that automatically arranges the children either left to right and top to bottom (for type() == \c FL_VERTICAL), or top to bottom and left to right (for type() == \c FL_HORIZONTAL), within the available size of the group.

class FLU_EXPORT GridGroup : public Fl_Group
{

 public:

  //! Normal FLTK constructor
  GridGroup( int x = 0, int y = 0, int w = 0, int h = 0, const char *l = 0 );

  //! \return the widget that is visibly above \b w in the group, or \c NULL if no such widget exists
  Fl_Widget *above( Fl_Widget* w );

  //! \return the widget that is visibly below \b w in the group, or \c NULL if no such widget exists
  Fl_Widget *below( Fl_Widget* w );

  //! Override of Fl_roup::layout()
  virtual void layout();

  //! Override of Fl_roup::draw()
  virtual void draw();

  //! \return the widget that is visibly to the left of \b w in the group, or \c NULL if no such widget exists
  Fl_Widget *left( Fl_Widget* w );

  //! \return the widget that is logically after \b w in the groups order, or \c NULL if no such widget exists
  Fl_Widget *next( Fl_Widget* w );

  //! Set the offset for where the first child starts
  inline void offset( int x, int y )
    { _offset[0] = x, _offset[1] = y; relayout(); }

  //! \return the x offset for where the first child starts
  inline int offset_x() const
    { return _offset[0]; }

  //! \return the y offset for where the first child starts
  inline int offset_y() const
    { return _offset[1]; }

  //! \return the widget that is logically before \b w in the groups order, or \c NULL if no such widget exists
  Fl_Widget *previous( Fl_Widget* w );

  //! \return the widget that is visibly to the right of \b w in the group, or \c NULL if no such widget exists
  Fl_Widget *right( Fl_Widget* w );

  //! Set the spacing between children
  inline void spacing( int x, int y )
    { _spacing[0] = x, _spacing[1] = y; relayout(); }

  //! \return the x spacing between children
  inline int spacing_x() const
    { return _spacing[0]; }

  //! \return the y spacing between children
  inline int spacing_y() const
    { return _spacing[1]; }

 protected:
  void _measure( int& W, int& H );

  void bbox( Fl_Rect& r );

  int _offset[2], _spacing[2], _type;
};

#endif
