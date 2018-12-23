//
// "$Id: ProgressBar.h 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Progress indicator with bar that fills up and text showing the
// job being done and expected time to go. Not fully implemented yet.
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

#ifndef fltk_ProgressBar_h
#define fltk_ProgressBar_h

#include "Widget.h"

namespace fltk {

class FL_API ProgressBar : public Widget {
protected:
  double mMin;
  double mMax;
  double mPresent;
  double mStep;
  bool mShowPct;
  Color mTextColor;
  void draw();
public:
  ProgressBar(int x, int y, int w, int h, const char *lbl = 0);
  void range(double min, double max, double step = 1)  { mMin = min; mMax = max; mStep = step; };
  void step(double step)		{ mPresent += step; redraw(); };
  double minimum()	{ return mMin; }
  double maximum()	{ return mMax; }
  void minimum(double nm) { mMin = nm; };
  void maximum(double nm) { mMax = nm; };
  double position()	{ return mPresent; }
  double step()		{ return mStep; }
  void position(double pos) 	{ mPresent = pos; redraw(); }
  void showtext(bool st)	{ mShowPct = st; }
  bool showtext()		{ return mShowPct; }
  void text_color(Color col)	{ mTextColor = col; }
  Color text_color() 	{ return mTextColor; }
};

}
#endif

