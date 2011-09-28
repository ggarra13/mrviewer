// $Id: Flu_Combo_Tree.h,v 1.6 2004/03/24 02:49:00 jbryan Exp $

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



#ifndef _FLU_COMBO_TREE_H
#define _FLU_COMBO_TREE_H

#include <fltk/InputBrowser.h>

#include <FLU/Flu_Enumerations.h>

//! @todo: Just like the fltk::InputBrowser widget except branches of the tree
//! can be selected.
class FLU_EXPORT Flu_Combo_Tree : public fltk::InputBrowser
{

public:
  //! Normal FLTK widget constructor
  Flu_Combo_Tree( int x, int y, int w, int h, const char *l = 0 );

  virtual ~Flu_Combo_Tree() {};


  virtual int popup(int x, int y, int w, int h);

  void value(int i) { fltk::InputBrowser::value(i); }
  int  value();

protected:
  void popup();
};

#endif
